#pragma once
template<typename K, typename V> class SkipList;
template<typename K,typename V>
class SkipListNode {
	template<typename K, typename V> friend class SkipList;
public:
	SkipListNode() {

	}
	SkipListNode(K key, V value) :_key(key), _value(value) {

	}
	SkipListNode(const SkipListNode* node) {
		_key = node->_key;
		_value = node->_value;
	}
	SkipListNode(const SkipListNode& node) {
		_key = node._key;
		_value = node._value;
	}
	K GetKey() {
		return _key;
	}
	V GetValue() {
		return _value;
	}
	~SkipListNode() {

	}
private:
	K _key;
	V _value;
	SkipListNode<K, V>* _next = nullptr;
	SkipListNode<K, V>* _down = nullptr;
	SkipListNode<K, V>* _prev = nullptr;
};