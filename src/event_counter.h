#ifndef __EVENT_COUNTER_iH
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
			printf(">%ld %ld %ld %ld\n", min, max, total, count);
		}
	};

	EventCounter() : queue_(N), partial_results_(), highest_timestamp_() {
	}

	void registerEvent(const Key &key, Value value, unsigned int timestamp);
	void query(const Key &key, unsigned int time_period, Result &result);
	void getTopKeys(std::vector<std::pair<Key, Result>> &results,
			unsigned int time_period, int n);

	void print() const {
		for (auto &x : partial_results_) {
			for (auto &y : x) {
				y.print();
			}
			printf("######\n");
		}
		printf("buffer %ld %ld:\n", queue_.start, queue_.end);
		for (int i = 0; i < N; ++i) {
			queue_[i][0].print();
		} printf("________\n");
	}

private:
	typedef std::array<Result, KeyCount> Results;
	void updatePeriod(const Results &results, int period);

	unsigned int highest_timestamp_;
	cbuf<Results, N+1> queue_;
	std::array<Results, sizeof(N)> partial_results_;
};

#endif
