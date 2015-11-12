#include "event_counter.h"

#include <cmath>

template<int N, typename Key, typename Value>
void EventCounter<N, Key, Value>::PartialResult::add(const Event &event) {
	this->values.insert(event.value);
	this->total += event.value;
	this->count += 1;
}

template<int N, typename Key, typename Value>
void EventCounter<N, Key, Value>::PartialResult::remove(const Event &event) {
	this->values.erase(event.value);
	this->total -= event.value;
	this->count -= 1;
}


template<int N, typename Key, typename Value>
void EventCounter<N, Key, Value>::updatePeriod(const Events &events, int period) {
	PartialResults &results = partial_results_[period];
	PartialResults &next_results = partial_results_[period + 1];

	for (const auto &event : events.data) {
		results[event.key].remove(event);
		next_results[event.key].add(event);
	}
}

template<int N, typename Key, typename Value>
void EventCounter<N, Key, Value>::registerEvent(const Key &key, Value value,
		unsigned int timestamp) {
	
	if (timestamp = highest_timestamp_ + 1) {
		for (int period = 0; period < (int)std::log2(N); period++) {
			Events &events = queue_[1 << period];
			updatePeriod(events, period);
		}
		queue_.push_front(Events{timestamp, {Event{key, value}}});
		queue_.pop_back();
	}
}

int main(void) {
	EventCounter<16> ec;	
	ec.registerEvent(std::string("aa"), 3, 1);
	ec.registerEvent(std::string("aa"), 3, 2);
	ec.registerEvent(std::string("aa"), 3, 3);
}
