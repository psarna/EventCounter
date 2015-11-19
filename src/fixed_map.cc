#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>

template<typename Value = int, int N = 5> 
class History_buffer {
public:

	History_buffer() : first_(-1), last_(-1), data_() {
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
		printf("[%d,%d]: ", first_, last_);
		int position = first_;
		for (int i = 0; i < N; i++) {
			printf("%d ", operator[](i));
		}
		printf("\n");
		/*
		for (const auto el : data_) {
			printf("%d[%d,%d] ", el.value, el.prev, el.next);
		}
		printf("\n===========");
		printf("\n");
		*/
	};

private:
	struct Element {
		Value value;
		int prev;
		int next;
		
		Element() {
			prev = -1;
			next = -1;
		}
	};
	
	int first_;
	int last_;
	std::array<Element, N> data_;
};


template<typename Key = char, typename Value = int, int N = 2>
class Fixed_map {
public:
	Fixed_map() : hist_(), data_() {
		/* add elem for every Value */
		for (Value i = 0; i < N; i++) {
			data_[i] = KeyValue('0' + i, i);
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
	void put(Key key) {
		/* first: ignore existing keys */
		auto it0 = std::lower_bound(data_.begin(), data_.end(), key);
		if (it0 != data_.end() && (*it0).key == key) {
			return;
		}

		int last = hist_.tail();
		printf("last[%d]", last);
		auto it = std::find_if(data_.rbegin(), data_.rend(),
				[last](const KeyValue& el) {
					return el.value == last;
				});
		assert(it != data_.rend());
		assert((*it).value == last);
		/* shift elements to the right removing old key */
		for (auto prev = it + 1; prev != data_.rend(); it++, prev++) {
			(*it) = (*prev);
		}
		/* There now is an extra space at data_.begin().
		   Now shift elements to the left until
		   space is in the place for new key. */
		auto it2 = data_.begin();
		auto next = it2 + 1;
		for(; (*next).key < key && next != data_.end(); it2++, next++) {
			(*it2) = (*next);
		}
		/* Now insert new key into the space */
		(*it2) = KeyValue(key, last);
		hist_.insert(last);
	};

	void print() const {
		for (const auto& el : data_) {
			printf("[%c,%d]", el.key, el.value);
		}
		printf("\n");
		hist_.print();
		printf("================\n");
	};

private:
	struct KeyValue {
		Key key;
		Value value;

		KeyValue(Key key='0', Value value=-1) : key(key), value(value) {
		};

		bool operator<(const Key& other) const {
			return key < other;
		};

		bool operator<(const KeyValue& other) const {
			return key < other.key;
		};
	};

	History_buffer<Value, N> hist_;
	std::array<KeyValue, N> data_;
};

int main() {
	History_buffer<> buf;
	buf.print();
	for (auto i : {1, 2, 2, 3, 4, 0, 2, 3, 1} ) {
		buf.insert(i);
		assert(buf.head() == i);
		buf.print();
	}
	Fixed_map<> map;
	for (auto i : {'0', '1', '3', '2', '1', '2', '3', '9'} ) {
		printf("m[%c]=%d\n", i, map.get(i));
		map.print();
		map.put(i);
		printf("m[%c]=%d\n", i, map.get(i));
		map.print();
		printf("############################\n");
	}
}
