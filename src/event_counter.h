#ifndef __EVENT_COUNTER_H
#define __EVENT_COUNTER_H

#include <array>
#include <limits>
#include <mutex>
#include <string>
#include <vector>

#include "cbuf.h"
#include "fixed_map.h"
#include "immortal_container.h"
#include "log2.h"
#include "mutex.h"
#include "sliding.h"

template<int N, int KeyCount, typename KeyRepresentation = std::array<char, 16>,
		typename Key = long, typename Value = long>
class EventCounter {
public:
	struct Result {
		Value min;
		Value max;
		Value total;
		long count;

		Result(Value value) : min(value), max(value), total(value), count(1) {
		}

		Result() : min(std::numeric_limits<Value>::max()), max(std::numeric_limits<Value>::min()),
			total(0), count(0) {
		}

		void add(const Result& result);
		void remove(const Result& result);
		void print() const {
			printf("(%s, %s, %ld, %ld)\n",
				min == std::numeric_limits<Value>::max() ? "X" : std::to_string(min).c_str(),
				max == std::numeric_limits<Value>::min() ? "X" : std::to_string(max).c_str(),
				total,
				count);
		}
	};

	struct PartialResult : public Result {
		typedef Sliding<Value, N/2, std::less<Value>> SlidingMax;
		typedef Sliding<Value, N/2, std::greater<Value>> SlidingMin;

		SlidingMin partial_min;
		cbuf<typename SlidingMin::pointer, N+1> min_pointers;
		SlidingMax partial_max;
		cbuf<typename SlidingMax::pointer, N+1> max_pointers;

		PartialResult(Value value) : Result(value) {
		}

		PartialResult() : Result() {
		}

		Value min() {
			return partial_min.top();
		}

		Value max() {
			return partial_max.top();
		}

		void add(const Result &result) {
			auto min_ptr = partial_min.update(result.min);
			min_pointers.put(min_ptr);
			auto max_ptr = partial_max.update(result.max);
			max_pointers.put(max_ptr);
			this->total += result.total;
			this->count += result.count;
		}

		void remove(const Result &result) {
			auto min_ptr = min_pointers.get();
			partial_min.erase(min_ptr);
			auto max_ptr = max_pointers.get();
			partial_max.erase(max_ptr);
			this->total -= result.total;
			this->count -= result.count;
		}
	};

	EventCounter() : queue_(N), partial_results_(), highest_timestamp_() {
	}

	void registerEvent(const KeyRepresentation &key, Value value, unsigned int timestamp);
	void query(const KeyRepresentation &key, unsigned int time_period, Result &result);
	void getTopKeys(std::vector<std::pair<KeyRepresentation, Result>> &results,
			unsigned int time_period, int n);

	void registerEvent(const Key &key, Value value, unsigned int timestamp);
	void query(const Key &key, unsigned int time_period, Result &result);
	void getTopKeys(std::vector<std::pair<Key, Result>> &results,
			unsigned int time_period, int n);

	void print(size_t max_len = 4) const {
		printf("Event Counter (highest timestamp = %d) {\n", highest_timestamp_);
		for (int i = 0; i < std::min(max_len, partial_results_.size()); ++i) {
			printf("\trange [%d, %d) : \n", (1 << i) - 1, (1 << (i + 1)) - 1);
			for (int j = 0; j < partial_results_[i].size(); ++j) {
				printf("\t\t%d: ", j);
				partial_results_[i][j].print();
			}
			printf("\n");
		}
		printf("\tBuffer (start = %ld, end = %ld) :\n", queue_.start, queue_.end);
		for (int i = 0; i < N; ++i) {
			printf("\t\t%d: ", i);
			queue_[i][0].print();
		}
		printf("}\n");
	}

private:
	typedef std::array<PartialResult, KeyCount> PartialResults;
	typedef std::array<Result, KeyCount> Results;

	void updatePeriod(const PartialResults &results, int period);
	void addToPeriod(const Key &key, const Result &result, int period);
	void removeFromPeriod(const Key &key, const Result &result, int period);
	void invalidateKey(const Key key);

	static PartialResults newResults(Key key, Value value) {
		PartialResults new_results = {};
		new_results[key] = {PartialResult(value)};
		return new_results;
	}

	unsigned int highest_timestamp_;
	cbuf<Results, N+1> queue_;
	std::array<PartialResults, 8 * sizeof(N)> partial_results_;
	Immortal<fixed_map<KeyRepresentation, Key, KeyCount>> key_map_;
	Mutex mutex_;
};

// Implementation

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::Result::add(const Result &other) {
	this->min = std::min(this->min, other.min);
	this->max = std::max(this->max, other.max);
	this->total += other.total;
	this->count += other.count;
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::Result::remove(const Result &other) {
	/* TODO: fixme
	if (other.min == this->min) {
		this->min = 0;
	}
	if (other.max == this->max) {
		this->max = 0;
	}*/
	this->total -= other.total;
	this->count -= other.count;
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::addToPeriod(
		const Key &key, const Result &result, int period) {
	PartialResults &curr_results = partial_results_[period];

	curr_results[key].add(result);
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::updatePeriod(
		const PartialResults &results, int period) {
	PartialResults &curr_results = partial_results_[period - 1];
	PartialResults &next_results = partial_results_[period];

	for (int i = 0; i < results.size(); ++i) {
		const Result &result = results[i];
		curr_results[i].remove(result);
		if (period < log2<N>()) {
			next_results[i].add(result);
		}
	}
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::invalidateKey(const Key key) {
	// remove from queue
	for (int i = queue_.start; i != queue_.end; i = (i + 1) % queue_.size) {
		queue_[i][key] = Result();
	}
	// remove from partial_results
	for (auto &partial : partial_results_) {
		partial[key] = Result();
	}
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::registerEvent(
		const KeyRepresentation &key_repr, Value value, unsigned int timestamp) {
	std::lock_guard<Mutex> lock(mutex_);
	Key key = key_map_.get(key_repr);
	if (key == -1) {
		key = key_map_.put(key_repr);
		// first - invalidate old entries.
		invalidateKey(key);
	}
	registerEvent(key, value, timestamp);
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::registerEvent(
		const Key &key, Value value, unsigned int timestamp) {
	// Case 1: Older event is registered
	if (timestamp <= highest_timestamp_ ) {
		// Edge case: Event is too old
		if (highest_timestamp_ - timestamp >= N) {
			printf("Ignoring old data: %d not in [%d, %d]\n", timestamp, highest_timestamp_ - N + 1, highest_timestamp_);
			return;
		}
		// Regular case: Event is insterted into the structure
		int queue_index = highest_timestamp_ - timestamp;
		Result &target = queue_[queue_index][key];
		target.add(Result(value));
		// Update period with newly added value
		addToPeriod(key, Result(value), log2(queue_index + 1));

	}

	// Case 2: Fresh event is registered
	if (timestamp > highest_timestamp_) {
		while (timestamp > highest_timestamp_ + 1) {
			highest_timestamp_++;
			// Add empty event to queue
			queue_.put(EventCounter::Results{});
			// Update partial results for all periods
			for (int period = 1; period <= log2<N>(); period++) {
				Results &results = queue_[(1 << period) - 1];
				updatePeriod(results, period);
			}
			// Remove the oldest record from the queue
			queue_.get();
		}
		//printf("timestamp = %u, highest_timestamp = %u, %u\n", timestamp, highest_timestamp_, highest_timestamp_ + 1);
		//assert(timestamp == highest_timestamp_ + 1);
		// Add newly registered event to queue
		queue_.put(EventCounter::newResults(key, value));
		// Update partial results for first period
		addToPeriod(key, Result(value), 0);
		// Update partial results for all periods
		for (int period = 1; period <= log2<N>(); period++) {
			Results &results = queue_[(1 << period) - 1];
			updatePeriod(results, period);
		}
		// Remove the oldest record from the queue
		queue_.get();

		highest_timestamp_ = timestamp;
	}
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::query(
		const KeyRepresentation &key_repr, unsigned int time_period, Result &result) {
	std::lock_guard<Mutex> lock(mutex_);
	Key key = key_map_.get(key_repr);
	if (key == -1){
		result = Result();
	} else {
		query(key, time_period, result);
	}
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::query(
		const Key &key, unsigned int time_period, Result &result) {
	for (int i = time_period - 1; i >= 0; --i) {
		Results &results = partial_results_[i];
		result.add(results[key]);
	}
}





#endif
