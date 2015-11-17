#include "event_counter.h"

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
void EventCounter<N, KeyCount, Key, Value>::addToPeriod(const Key &key, const Result &result, int period) {
	printf("Adding a result to period %d:\n", period);
	Results &curr_results = partial_results_[period];

	curr_results[key].add(result);
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::updatePeriod(const Results &results, int period) {
	printf("Updating period %d with results:\n", period);
	for (auto &result : results) {
		result.print();
	}

	Results &curr_results = partial_results_[period - 1];
	Results &next_results = partial_results_[period];

	for (int i = 0; i < results.size(); ++i) {
		const Result &result = results[i];
		curr_results[i].remove(result);
		if (period < log2<N>()) {
			next_results[i].add(result);
		}
	}
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::registerEvent(const Key &key, Value value,
		unsigned int timestamp) {

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
	if (timestamp == highest_timestamp_ + 1) {
		printf("Registering event: [%ld, %ld]\n", key, value);
		// Add newly registered event to queue
		queue_.put(EventCounter::newResults(key, value));
		// Update partial results for first period
		addToPeriod(key, Result(value), 0);
		// Update partial results for all periods
		for (int period = 1; period <= log2<N>(); period++) {
			printf("Updating for queue element %d\n", (1 << period) - 1);
			Results &results = queue_[(1 << period) - 1];
			updatePeriod(results, period);
		}
		// Remove the oldest record from the queue
		queue_.get();

		highest_timestamp_ = timestamp;
	}
	if (timestamp > highest_timestamp_ + 1) {
		printf("TODO: Implement me\n");
	}
}

template<int N, int KeyCount, typename Key, typename Value>
void EventCounter<N, KeyCount, Key, Value>::query(const Key &key, unsigned int time_period, Result &result) {
	for (int i = time_period - 1; i >= 0; --i) {
		Results &results = partial_results_[i];
		result.add(results[key]);
	}
}

typedef EventCounter<8, 2> Counter;
typedef Counter::Result Result;

int main(void) {
	Counter ec;
	printf("\n\n=== registerEvent() tests for new events ===\n\n");
	for (int i = 0; i < 102; ++i) {
		ec.registerEvent(0, i + 3, i + 1);
	}
	ec.print();

	printf("\n\n=== query() tests ===\n\n");
	Result result;

	ec.print();
	ec.query(0, 2, result);
	printf("Query result: ");
	result.print();

	Result res2;
	ec.query(0, 1, res2);

	printf("Query result: ");
	res2.print();

	printf("\n\n=== registerEvent() tests for old events ===\n\n");
	ec.print();
	ec.registerEvent(0, 5, 5);
	ec.registerEvent(0, 1000, 99);
	ec.print();

	Result res3;
	ec.query(0, 3, res3);
	printf("Query result: ");
	res3.print();
}
