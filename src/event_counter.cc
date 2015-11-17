#include "event_counter.h"

#include <cmath>
#include <cstdio>

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::Result::add(const Result &other) {
	printf("Adding:\n");
	printf("\t");
	print();
	printf("\t");
	other.print();
	this->min = std::min(this->min, other.min);
	this->max = std::max(this->max, other.max);
	this->total += other.total;
	this->count += other.count;
	printf("\t------------\n\t");
	print();
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::Result::remove(const Result &other) {
	printf("Removing:\n");
	printf("\t");
	print();
	printf("\t");
	other.print();
	/* TODO: fixme
	if (other.min == this->min) {
		this->min = 0;
	}
	if (other.max == this->max) {
		this->max = 0;
	}*/
	this->total -= other.total;
	this->count -= other.count;
	printf("\t------------\n\t");
	print();
}


template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::updatePeriod(const Results &results, int period) {
	printf("Updating period %d with results:\n", period);
	for (auto &result : results) {
		result.print();
	}

	// Edge case 1: Adding to first partial result
	if (period == 0) {
		Results &curr_results = partial_results_[0];

		for (int i = 0; i < results.size(); ++i) {
			const Result &result = results[i];
			curr_results[i].add(result);
		}
		return;
	}

	// Edge case 1: Removing from last partial result
	if (period == log2<N>()) {
		Results &curr_results = partial_results_[period - 1];

		for (int i = 0; i < results.size(); ++i) {
			const Result &result = results[i];
			curr_results[i].remove(result);
		}
		return;
	}

	// Regular case: Removing from previous results,
	// adding to current results
	Results &curr_results = partial_results_[period - 1];
	Results &next_results = partial_results_[period];

	for (int i = 0; i < results.size(); ++i) {
		const Result &result = results[i];
		curr_results[i].remove(result);
		next_results[i].add(result);
	}
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::registerEvent(const Key &key, Value value,
		unsigned int timestamp) {
	if (timestamp == highest_timestamp_ + 1) {
		// Add newly registered event to queue
		queue_.put(EventCounter::newResults(key, value));
		// Update partial results for all periods
		for (int period = 0; period < log2<N>(); period++) {
			Results &results = queue_[(1 << period) - 1];
			updatePeriod(results, period);
		}
		// Remove the oldest record from the queue and update partial results
		Results &last_results = queue_.get();
		updatePeriod(last_results, log2<N>());
		highest_timestamp_ = timestamp;
	}
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::query(const Key &key, unsigned int time_period, Result &result) {
	for (int i = time_period - 1; i >= 0; --i) {
		Results &results = partial_results_[i];
		result.add(results[key]);
	}
}

typedef EventCounter<4, 2> Counter;
typedef Counter::Result Result;

int main(void) {
	Counter ec;
	ec.print();
	ec.registerEvent(0, 3, 1);
	ec.print();
	ec.registerEvent(0, 4, 2);
	ec.print();
	ec.registerEvent(0, 5, 3);
	ec.print();
	ec.registerEvent(0, 6, 4);
	ec.print();
	ec.registerEvent(0, 7, 5);
	ec.print();
	exit(1);
	Result result;

	ec.print();
	ec.query(0, 2, result);
	ec.print();
	printf("-----\n");

	printf("Query:\n");
	result.print();
	Result res2;
	ec.query(0, 1, res2);

	printf("Query:\n");
	res2.print();
}
