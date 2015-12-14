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

#define key_size 16
typedef std::array<char, key_size> KeyType;

std::string to_string(const KeyType &key){
	return std::string(key.data(), key.size());
}

KeyType to_key(const std::string &value) {
	KeyType ret;
	strncpy(ret.data(), value.c_str(), ret.size());
	return ret;
}

template<int N, int KeyCount, typename KeyRepresentation = KeyType,
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

		void print() const {
			printf("(%s, %s, %ld, %ld)\n",
				min == std::numeric_limits<Value>::max() ? "X" : std::to_string(min).c_str(),
				max == std::numeric_limits<Value>::min() ? "X" : std::to_string(max).c_str(),
				total,
				count);
		}
	};

	struct AccumulatedResult : public Result {
		typedef Sliding<Value, N, std::less<Value>, std::numeric_limits<Value>::min()> SlidingMax;
		typedef Sliding<Value, N, std::greater<Value>, std::numeric_limits<Value>::max()> SlidingMin;

		Immortal<SlidingMin> accumulated_min;
		cbuf<typename SlidingMin::pointer, N+1> min_pointers;
		Immortal<SlidingMax> accumulated_max;
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

		void add(AccumulatedResult &result) {
			if (!min_pointers.full()) {
				auto min_ptr = accumulated_min.update(result.min());
				min_pointers.put(min_ptr);
			}
			if (!max_pointers.full()) {
				auto max_ptr = accumulated_max.update(result.max());
				max_pointers.put(max_ptr);
			}
			this->total += result.total;
			this->count += result.count;
		}

		void remove(const AccumulatedResult &result) {
			if (!min_pointers.empty()) {
				auto min_ptr = min_pointers.get();
				accumulated_min.erase(min_ptr);
			}
			if (!max_pointers.empty()) {
				auto max_ptr = max_pointers.get();
				accumulated_max.erase(max_ptr);
			}
			this->total -= result.total;
			this->count -= result.count;
		}

		void add(const Result &result) {
			if (!min_pointers.full()) {
				auto min_ptr = accumulated_min.update(result.min);
				min_pointers.put(min_ptr);
			}
			if (!max_pointers.full()) {
				auto max_ptr = accumulated_max.update(result.max);
				max_pointers.put(max_ptr);
			}
			this->total += result.total;
			this->count += result.count;
		}

		void remove(const Result &result) {
			if (!min_pointers.empty()) {
				auto min_ptr = min_pointers.get();
				accumulated_min.erase(min_ptr);
			}
			if (!max_pointers.empty()) {
				auto max_ptr = max_pointers.get();
				accumulated_max.erase(max_ptr);
			}
			this->total -= result.total;
			this->count -= result.count;
		}
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
			unsigned int time_period, int n) {
		std::vector<std::pair<Key, Result>> rets;
		std::lock_guard<Mutex> lock(mutex_);
		results.resize(0);
		getTopKeys(rets, time_period, n);
		for (auto &pair : rets) {
			const KeyRepresentation &key_repr = key_map_.getKey(pair.first);
			results.emplace_back(std::make_pair(key_repr, pair.second));
		}
	}

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
		AccumulatedResult ret;
		for (int i = time_period - 1; i >= 0; --i) {
			AccumulatedResults &results = accumulated_results_[i];
			ret.add(results[key]);
		}
		result.min = ret.min();
		result.max = ret.max();
		result.total = ret.total;
		result.count = ret.count;
	}

	void getTopKeys(std::vector<std::pair<Key, Result>> &results,
			unsigned int time_period, int n) {
		// 1. count most used keys
		std::vector<std::pair<int, Key>> key_counts(KeyCount);
		for (Key key = 0; key < KeyCount; key++) {
			key_counts[key] = std::make_pair(0, key);
		}
		for (int i = time_period - 1; i >= 0; --i) {
			AccumulatedResults &accum_results = accumulated_results_[i];
			for (Key key = 0; key < KeyCount; key++) {
				key_counts[key].first += accum_results[key].count;
			}
		}
		// 2. get array with n most used keys
		std::partial_sort(key_counts.begin(), key_counts.begin() + n, key_counts.end(),
				[](const std::pair<int, Key> &a, const std::pair<int, Key> &b) { return a > b;});
		key_counts.resize(n);
		// 3. calculate accumulated results in ret
		std::vector<AccumulatedResult> ret(n);
		for (int i = time_period - 1; i >= 0; --i) {
			AccumulatedResults &accum_results = accumulated_results_[i];
			for (Key j = 0; j < n; j++) {
				Key key = key_counts[j].second;
				ret[j].add(accum_results[key]);
			}
		}
		// 4. save results
		results.resize(n);
		for (Key j = 0; j < n; j++) {
			Key key = key_counts[j].second;
			Result &result = results[j].second;
			AccumulatedResult &accum_result = ret[j];

			// first save key
			results[j].first = key;
			// then save result
			result.min = accum_result.min();
			result.max = accum_result.max();
			result.total = accum_result.total;
			result.count = accum_result.count;
		}
	}

	void print(size_t max_len = 4) {
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
		printf("\tFixed_map:\n");
		for (Key i = 0; i < KeyCount; ++i) {
			KeyRepresentation val = key_map_.getKey(i);
			printf("\t\tM[%ld] = %s\n", i, val.data());
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
