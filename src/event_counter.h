#ifndef __EVENT_COUNTER_H
#define __EVENT_COUNTER_H

#include <array>
#include <limits>
#include <string>
#include <vector>

#include "cbuf.h"

template<int N, int KeyCount, typename Key = long, typename Value = long>
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
			printf("(%ld, %ld, %ld, %ld)\n",
					min == std::numeric_limits<Value>::max() ? 0 : min,
					max == std::numeric_limits<Value>::min() ? 0 : max,
					total,
					count);
		}
	};

	EventCounter() : queue_(N), partial_results_(), highest_timestamp_() {
	}

	void registerEvent(const Key &key, Value value, unsigned int timestamp);
	void query(const Key &key, unsigned int time_period, Result &result);
	void getTopKeys(std::vector<std::pair<Key, Result>> &results,
			unsigned int time_period, int n);

	void print() const {
		printf("Event Counter {\n");
		for (int i = 0; i < partial_results_.size(); ++i) {
			printf("\trange [%d, %d) : \n", (1 << i) - 1, (1 << (i + 1)) - 1);
			for (int j = 0; j < partial_results_[i].size(); ++j) {
				printf("\t\t%d: ", j);
				partial_results_[i][j].print();
			}
			printf("\n");
		}
		printf("\tBuffer (start = %ld, end = %ld) :\n", queue_.start, queue_.end);
		for (int i = 0; i < N; ++i) {
			printf("\t\t %d: ", i);
			queue_[i][0].print();
		}
		printf("}\n");
	}

private:
	typedef std::array<Result, KeyCount> Results;

	void updatePeriod(const Results &results, int period);

	static Results newResults(Key key, Value value) {
		Results new_results = {};
		new_results[key] = {Result(value)};
		return new_results;
	}

	unsigned int highest_timestamp_;
	cbuf<Results, N+1> queue_;
	std::array<Results, sizeof(N)> partial_results_;
};

#endif
