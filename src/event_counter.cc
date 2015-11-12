#include "event_counter.h"

void EventCounter::Result::add(const Event &event) {
	this->min = std::min(this->min, event.value);
	this->max = std::max(this->max, event.value);
	this->total += event.value;
	this->count += 1;
}



void EventCounter::updatePeriod(const Event &event, int period) {
	Results &results = partial_results_[period];
	Results &next_results = partial_results_[period + 1];

	results[event.key].remove(event);
	next_results[event.key].add(event);
}

void EventCounter::registerEvent(const Key &key, Value value,
		unsigned int /*timestamp*/) {
	
}

int main(void) {
	
}
