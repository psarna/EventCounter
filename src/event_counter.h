#ifndef __EVENT_COUNTER_H
#define __EVENT_COUNTER_H

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

template<int N, typename Key = std::string, typename Value = long>
class EventCounter {
	struct Event {
		Key key;
		Value value;
	};

	struct Result {
		long min;
		long max;
		long total;
		long count;

		void add(const Event &event);
		void remove(const Event &event);
	};

typedef std::unordered_map<Key, Result> Results;

public:
	EventCounter() : queue_(N), partial_results_() {
	}

	void registerEvent(const Key &key, Value value, unsigned int timestamp);
	void query(const Key &key, unsigned int time_period, Result &result);
	void getTopKeys(std::vector<std::pair<Key, Result>> &results,
			unsigned int time_period, int n);

private:
	void updatePeriod(const Event &event, int period);

	std::deque<Event> queue_;
	std::array<Results, sizeof(N)> partial_results_;
};

#endif
