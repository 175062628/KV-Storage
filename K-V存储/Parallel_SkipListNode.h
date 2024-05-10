#pragma once
#include"ReadWriteLock.h"
template<typename K, typename V, K MinK> class Parallel_SkipList;
template<typename K, typename V>
class Parallel_SkipListNode {
	template<typename K, typename V, K MinK> friend class Parallel_SkipList;
	friend class ReadWriteLock;
public:
	typedef ReadWriteLock		Latch;
public:
	Parallel_SkipListNode(K key) :_key(key), _exist(true) {

	}
	Parallel_SkipListNode(K key, V value) :_key(key), _value(value), _exist(true){

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
	bool Exist() {
		return _exist;
	}
	void Delete() {
		_exist = false;
	}
	void Insert() {
		_exist = true;
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
	bool _exist;
};