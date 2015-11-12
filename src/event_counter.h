#ifndef __EVENT_COUNTER_H
#define __EVENT_COUNTER_H

#include <deque>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct Result {
	long min;
	long max;
	long total;
	long count;
};

template<int N, typename Key = std::string, typename Value = long>
class EventCounter {
	struct Event {
		Key key;
		Value value;
	};

	struct Events {
		unsigned int timestamp;
		std::vector<Event> data;
	};


	struct PartialResult {
		std::set<Value> values;
		Value total;
		long count;

		void add(const Event &event);
		void remove(const Event &event);
	};

typedef std::unordered_map<Key, PartialResult> PartialResults;

public:
	EventCounter() : queue_(N+1), partial_results_(), highest_timestamp_() {
	}

	void registerEvent(const Key &key, Value value, unsigned int timestamp);
	void query(const Key &key, unsigned int time_period, Result &result);
	void getTopKeys(std::vector<std::pair<Key, Result>> &results,
			unsigned int time_period, int n);

private:
	void updatePeriod(const Events &events, int period);

	unsigned int highest_timestamp_;
	std::deque<Events> queue_;
	std::array<PartialResults, sizeof(N)> partial_results_;
};

#endif
