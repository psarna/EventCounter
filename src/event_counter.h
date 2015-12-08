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

		void add(const Result& other) {
			this->min = std::min(this->min, other.min);
			this->max = std::max(this->max, other.max);
			this->total += other.total;
			this->count += other.count;
		}


		void remove(const Result& other) {
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

		void print() const {
			printf("(%s, %s, %ld, %ld)\n",
				min == std::numeric_limits<Value>::max() ? "X" : std::to_string(min).c_str(),
				max == std::numeric_limits<Value>::min() ? "X" : std::to_string(max).c_str(),
				total,
				count);
		}
	};

	struct AccumulatedResult : public Result {
		typedef Sliding<Value, N/2, std::less<Value>> SlidingMax;
		typedef Sliding<Value, N/2, std::greater<Value>> SlidingMin;

		SlidingMin accumulated_min;
		cbuf<typename SlidingMin::pointer, N+1> min_pointers;
		SlidingMax accumulated_max;
		cbuf<typename SlidingMax::pointer, N+1> max_pointers;

		AccumulatedResult(Value value) : Result(value) {
		}

		AccumulatedResult() : Result() {
		}

		Value min() {
			return accumulated_min.top();
		}

		Value max() {
			return accumulated_max.top();
		}

		/*void add(const Result &result) {
			auto min_ptr = accumulated_min.update(result.min);
			min_pointers.put(min_ptr);
			auto max_ptr = accumulated_max.update(result.max);
			max_pointers.put(max_ptr);
			this->total += result.total;
			this->count += result.count;
		}

		void remove(const Result &result) {
			auto min_ptr = min_pointers.get();
			accumulated_min.erase(min_ptr);
			auto max_ptr = max_pointers.get();
			accumulated_max.erase(max_ptr);
			this->total -= result.total;
			this->count -= result.count;
		}*/
	};

	EventCounter() : queue_(N), accumulated_results_(), highest_timestamp_() {
	}

	void registerEvent(const KeyRepresentation &key_repr, Value value, unsigned int timestamp) {
		std::lock_guard<Mutex> lock(mutex_);
		Key key = key_map_.get(key_repr);
		if (key == -1) {
			key = key_map_.put(key_repr);
			// first - invalidate old entries.
			invalidateKey(key);
		}
		registerEvent(key, value, timestamp);
	}

	void getTopKeys(std::vector<std::pair<KeyRepresentation, Result>> &results,
			unsigned int time_period, int n);

	void registerEvent(const Key &key, Value value, unsigned int timestamp) {
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
				assert(timestamp == highest_timestamp_ + 1);
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

	void query(const KeyRepresentation &key_repr, unsigned int time_period, Result &result) {
		std::lock_guard<Mutex> lock(mutex_);
		Key key = key_map_.get(key_repr);
		if (key == -1){
			result = Result();
		} else {
			query(key, time_period, result);
		}
	}

	void query(const Key &key, unsigned int time_period, Result &result) {
		for (int i = time_period - 1; i >= 0; --i) {
			AccumulatedResults &results = accumulated_results_[i];
			result.add(results[key]);
		}
	}

	void getTopKeys(std::vector<std::pair<Key, Result>> &results,
			unsigned int time_period, int n);

	void print(size_t max_len = 4) const {
		printf("Event Counter (highest timestamp = %d) {\n", highest_timestamp_);
		for (int i = 0; i < std::min(max_len, accumulated_results_.size()); ++i) {
			printf("\trange [%d, %d) : \n", (1 << i) - 1, (1 << (i + 1)) - 1);
			for (int j = 0; j < accumulated_results_[i].size(); ++j) {
				printf("\t\t%d: ", j);
				accumulated_results_[i][j].print();
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
	typedef std::array<AccumulatedResult, KeyCount> AccumulatedResults;
	typedef std::array<Result, KeyCount> Results;

	void updatePeriod(const Results &results, int period) {
		AccumulatedResults &curr_results = accumulated_results_[period - 1];
		AccumulatedResults &next_results = accumulated_results_[period];

		for (int i = 0; i < results.size(); ++i) {
			const Result &result = results[i];
			curr_results[i].remove(result);
			if (period < log2<N>()) {
				next_results[i].add(result);
			}
		}
	}

	void addToPeriod(const Key &key, const Result &result, int period) {
		AccumulatedResults &curr_results = accumulated_results_[period];

		curr_results[key].add(result);
	}

	void removeFromPeriod(const Key &key, const Result &result, int period);

	void invalidateKey(const Key key) {
		// remove from queue
		for (int i = queue_.start; i != queue_.end; i = (i + 1) % queue_.size) {
			queue_[i][key] = Result();
		}
		// remove from partial_results
		for (auto &accumulated : accumulated_results_) {
			accumulated[key] = AccumulatedResult();
		}
	}

	static Results newResults(Key key, Value value) {
		Results new_results = {};
		new_results[key] = {Result(value)};
		return new_results;
	}

	unsigned int highest_timestamp_;
	cbuf<Results, N+1> queue_;
	std::array<AccumulatedResults, 8 * sizeof(N)> accumulated_results_;
	Immortal<fixed_map<KeyRepresentation, Key, KeyCount>> key_map_;
	Mutex mutex_;
};

#endif
