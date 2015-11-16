#include "event_counter.h"

#include <cmath>
#include <cstdio>

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::Result::add(const Result &other) {
	printf("add\n");
	print();
	other.print();
	this->min = std::min(this->min, other.min);
	this->max = std::max(this->max, other.max);
	this->total += other.total;
	this->count += other.count;
	print();
	printf("--\n");
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::Result::remove(const Result &other) {
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


template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::updatePeriod(const Results &results, int period) {
	if (period == 0) {
		Results &curr_results = partial_results_[period];

		for (int i = 0; i < results.size(); ++i) {
			const Result &result = results[i];
			result.print();
			curr_results[i].add(result);
		}
		return;
	}

	Results &curr_results = partial_results_[period - 1];
	Results &next_results = partial_results_[period];

	for (int i = 0; i < results.size(); ++i) {
		const Result &result = results[i];
		result.print();
		curr_results[i].remove(result);
		next_results[i].add(result);
	}
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::registerEvent(const Key &key, Value value,
		unsigned int timestamp) {
	print();
	printf("Timestamp = %u\n; highest = %u\n", timestamp, highest_timestamp_);
	if (timestamp = highest_timestamp_ + 1) {
		Results new_results = {};
		new_results[key] = {Result(value)};
		queue_.put(new_results);
		for (int period = 0; period < (int)std::log2(N); period++) {
			printf("%d\n", 1 << period);
			Results &results = queue_[(1 << period) - 1];
			printf("Results\n"); results[0].print();
			updatePeriod(results, period);
		}
		queue_.get();
		highest_timestamp_ = timestamp;
	}
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::query(const Key &key, unsigned int time_period, Result &result) {
	for (int i = time_period - 1; i >= 0; --i) {
		Results &results = partial_results_[i];
		printf("Partial %d: \n", i);
		partial_results_[i][key].print();
		result.add(results[key]);
		result.print();
		printf("----\n");
		
	}
}

int main(void) {
	EventCounter<4, 1> ec;	
	ec.registerEvent(0, 3, 1);
	ec.registerEvent(0, 4, 2);
	ec.registerEvent(0, 5, 3);
	EventCounter<4, 1>::Result result;
	printf("---\n");
	ec.print();
	printf("---\n");
	ec.query(0, 2, result);

	printf("Query:\n");
	result.print();
	EventCounter<4, 1>::Result res2;
	ec.query(0, 1, res2);

	printf("Query:\n");
	res2.print();
}
