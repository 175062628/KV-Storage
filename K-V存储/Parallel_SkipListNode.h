#pragma once
#include"ReadWriteDeleteLock.h"
template<typename K, typename V> class Parallel_SkipList;
template<typename K, typename V>
class Parallel_SkipListNode {
	template<typename K, typename V> friend class Parallel_SkipList;
	friend class ReadWriteDeleteLock;
public:
	typedef ReadWriteDeleteLock		Latch;
public:
	Parallel_SkipListNode() {

	}
	Parallel_SkipListNode(K key, V value) :_key(key), _value(value) {

	}
	Parallel_SkipListNode(const Parallel_SkipListNode* node) {
		_key = node->_key;
		_value = node->_value;
	}
	Parallel_SkipListNode(const Parallel_SkipListNode& node) {
		_key = node._key;
		_value = node._value;
	}
	K GetKey() {
		return _key;
	}
	V GetValue() {
		return _value;
	}
	~Parallel_SkipListNode() {

	}
private:
	K _key;
	V _value;
	Parallel_SkipListNode<K, V>* _next = nullptr;
	Parallel_SkipListNode<K, V>* _down = nullptr;
	Parallel_SkipListNode<K, V>* _prev = nullptr;
	Latch _latch;
	bool _exist = true;
};