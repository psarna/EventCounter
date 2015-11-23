#include "event_counter.h"

#include "mutex.h"

#include <cstdio>

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::Result::add(const Result &other) {
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

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::Result::remove(const Result &other) {
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

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::addToPeriod(
		const Key &key, const Result &result, int period) {
	printf("Adding a result to period %d:\n", period);
	Results &curr_results = partial_results_[period];

	curr_results[key].add(result);
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::updatePeriod(
		const Results &results, int period) {
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
	Key key = key_map_.get(key_repr);
	if (key == -1) {
		key = key_map_.put(key_repr);
		printf("Key is %ld\n", key);
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
		printf("Registering event: [%ld, %ld]\n", key, value);
		while (timestamp > highest_timestamp_ + 1) {
			highest_timestamp_++;
			// Add empty event to queue
			queue_.put(EventCounter::Results{});
			// Update partial results for all periods
			for (int period = 1; period <= log2<N>(); period++) {
				printf("Updating for queue element %d\n", (1 << period) - 1);
				Results &results = queue_[(1 << period) - 1];
				updatePeriod(results, period);
			}
			// Remove the oldest record from the queue
			queue_.get();
		}
		assert(timestamp == highest_timestamp_ + 1);
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
}

template<int N, int KeyCount, typename KeyRepresentation, typename Key, typename Value>
void EventCounter<N, KeyCount, KeyRepresentation, Key, Value>::query(
		const KeyRepresentation &key_repr, unsigned int time_period, Result &result) {
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

	ec.registerEvent(0, 20000, 105);
	ec.print();

	Result res4;
	ec.query(0, 3, res4);
	printf("Query result: ");
	res4.print();

	std::array<char, 16> key1 = {'a', 'b', 'c'};
	std::array<char, 16> key0 = {'a', 'b', 'd'};
	std::array<char, 16> key00 = {'a', 'b', 'a'};

	ec.registerEvent(key1, 4321, 106);
	ec.registerEvent(key0, 1234, 107);
	ec.registerEvent(key1, 9876, 106);

	Mutex mutex;
	mutex.lock();
	ec.registerEvent(key00, 5555, 108);
	ec.print();
	mutex.unlock();

}
