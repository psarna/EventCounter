#ifndef __fixed_map_H
#define __fixed_map_H

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>

template<typename Value = int, int N = 5> 
class lru_buffer {
public:

	lru_buffer() : first_(0), last_(N - 1) {
		/* initialize full */
		for (int i = 0; i < N; i++) {
			int prev = i - 1;
			int next = i + 1 == N ? -1 : i + 1;
			data_[i] = Element(i, prev, next); 
		}
	}

	/* constant time */
	void insert(Value value) {
		if (first_ == value) {
			return;
		}
		Element& curr = data_[value];
		/* remove from list */
		if (curr.prev != -1 ) {
			/* make link from previous to next element */
			data_[curr.prev].next = curr.next;
		}
		if (curr.next != -1 ) {
			/* make link from next to previous element */
			data_[curr.next].prev = curr.prev;
		}
		if (last_ == value) {
			/* removing last element */
			last_ = curr.prev;
		}
		/* insert at the start */
		curr.prev = -1;
		curr.next = first_;
		curr.value = value;
		if (first_ != -1) {
			/* make link form old first to current first */
			data_[first_].prev = value;
		}
		if (last_ == -1) {
			last_ = value;
		}
		first_ = value;
	}

	/* constant time */
	Value head() const {
		return data_[first_].value;
	}
	
	/* constant time */
	Value tail() const{
		return data_[last_].value;
	}

	/* linear time */
	Value operator[](int position) const {
		int curr = first_;
		while (curr != -1) {
			const Element& elem = data_[curr];
			if (position == 0) {
				return elem.value;
			}
			position--;
			curr = elem.next;
		}
		return -1;
	}

	void print() const {
		printf("lru_buffer {");
		printf("[%d,%d]: ", first_, last_);
		int position = first_;
		for (int i = 0; i < N; i++) {
			printf("%d ", operator[](i));
		}
		printf("};\n");
	};

private:
	struct Element {
		Value value;
		int prev;
		int next;
		
		Element(Value value = 0, int prev = -1, int next = -1)
			: value(value),
			  prev(prev),
			  next(next) {
		}
	};
	
	int first_;
	int last_;
	std::array<Element, N> data_;
};


template<typename Key = char, typename Value = int, int N = 2>
class fixed_map {
public:
	typedef Key key_type;
	typedef Value value_type;

	fixed_map() : hist_(), data_() {
		/* initialize values for elements */
		for (Value i = 0; i < N; i++) {
			data_[i].value = i;
		}
	};
	
	/* log(number of keys) */
	Value get(Key key) {
		const auto& it = std::lower_bound(data_.begin(),
				data_.end(), key);
		if (it == data_.end() || it->key != key) {
			return -1;
		}
		Value result = it->value;
		hist_.insert(result);
		return result;
	};

	/* linear by the number of keys */
	Value put(Key key) {
		/* first: ignore existing keys */
		auto it0 = std::lower_bound(data_.begin(), data_.end(), key);
		if (it0 != data_.end() && it0->key == key) {
			return it0->value;
		}

		int last_value = hist_.tail();
		printf("last_value[%d]\n", last_value);
		// 1. get iterator to remove
		auto it_remove = std::find_if(data_.begin(), data_.end(),
				[last_value](const KeyValue& el) {
					return el.value == last_value;
				});
		assert(it_remove != data_.end());
		assert((*it_remove).value == last_value);
		// 2. get iterator to new place
		auto it_insert = std::lower_bound(data_.begin(), data_.end(), key);
		// 3. shift between
		if (key < it_remove->key) {
			std::move_backward(it_insert, it_remove, it_remove + 1);
		} else { /* key > it_remove->key */
			std::move(it_remove + 1, it_insert, it_remove);
			it_insert--;
		}
		// 4. insert new
		(*it_insert) = KeyValue(key, last_value);
		hist_.insert(last_value);
		return last_value;
	};

	void print() const {
		printf("fixed_map {\n");
		for (const auto& el : data_) {
			Key key = el.key;
			if (key[15] == std::numeric_limits<Key>::max()) {
				key = '#';
			}
			printf("\t[%c,%d]", key, el.value);
		}
		printf("\n");
		hist_.print();
		printf("}\n");
	};

private:
	struct KeyValue {
		Key key;
		Value value;
		
		KeyValue() : key{}, value(-1) {
			for (auto &c : key) {
				c = std::numeric_limits<char>::max();
			}
		}

		/* construct as the highest possible Key value */
		KeyValue(Key key, Value value)
			: key(key),
			  value(value) {
		};

		bool operator<(const Key& other) const {
			return key < other;
		};

		bool operator<(const KeyValue& other) const {
			return key < other.key;
		};
	};

	lru_buffer<Value, N> hist_;
	std::array<KeyValue, N> data_;
};

#endif
