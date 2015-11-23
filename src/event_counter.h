#ifndef __EVENT_COUNTER_H
#define __EVENT_COUNTER_H

#include <array>
#include <limits>
#include <string>
#include <vector>

#include "cbuf.h"
#include "fixed_map.h"
#include "log2.h"

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
	typedef std::array<Result, KeyCount> Results;

	void updatePeriod(const Results &results, int period);
	void addToPeriod(const Key &key, const Result &result, int period);
	void removeFromPeriod(const Key &key, const Result &result, int period);

	static Results newResults(Key key, Value value) {
		Results new_results = {};
		new_results[key] = {Result(value)};
		return new_results;
	}

	unsigned int highest_timestamp_;
	cbuf<Results, N+1> queue_;
	std::array<Results, 8 * sizeof(N)> partial_results_;
	fixed_map<KeyRepresentation, Key, KeyCount> key_map_;
};

#endif
