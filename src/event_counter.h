#ifndef __EVENT_COUNTER_H
#define __EVENT_COUNTER_H

#include <string>
#include <vector>

struct Result {
	long min;
	long max;
	long total;
	long count;
};

class EventCounter {
	typedef ::std::string Key;
	typedef long Value;
public:
	void registerEvent(const Key &key, Value value, unsigned int timestamp);
	void query(const Key &key, unsigned int time_period, Result &result);
	void getTopKeys(std::vector<std::pair<Key, Result>> &results,
			unsigned int time_period, int n);

private:
};

#endif
