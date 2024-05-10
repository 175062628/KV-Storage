#pragma once
#include<vector>
#include<iostream>
#include<mutex>
#include"Parallel_SkipListNode.h"
template<typename K, typename V, K MinK>
class Parallel_SkipList {
	template<typename K, typename V> friend class Parallel_SkipListNode;
public:
	typedef Parallel_SkipListNode<K, V>			Node;
	typedef Parallel_SkipListNode<K, V>*		NodePtr;
	typedef const K&							const_K_ref;
	typedef const V&							const_V_ref;
public:
	Parallel_SkipList() {

	}
	Parallel_SkipList(std::vector<std::pair<K, V>>& data) {
		for (auto& [key, value] : data) {
			Insert(key, value);
		}
	}
	Parallel_SkipList(const Parallel_SkipList& Parallel_SkipList) {
		this->_levelCount = Parallel_SkipList._levelCount;
		this->_head = Parallel_SkipList._head;
		this->POSSIBILITY = Parallel_SkipList.POSSIBILITY;
		this->MAXLEVEL = Parallel_SkipList.MAXLEVEL;
	}
	Parallel_SkipList& operator=(const Parallel_SkipList& Parallel_SkipList) {
		if (this != &Parallel_SkipList) {
			this->_levelCount = Parallel_SkipList._levelCount;
			this->_head = Parallel_SkipList._head;
			this->POSSIBILITY = Parallel_SkipList.POSSIBILITY;
			this->MAXLEVEL = Parallel_SkipList.MAXLEVEL;
		}
		return *this;
	}

	bool Insert(const_K_ref key, const_V_ref value) {
		_latch.lock();
		// 插入第一个节点
		if (!_head) {
			_head = new Node(MinK);
			_head->Delete();
			_head->_next = new Node(key, value);
			_latch.unlock();
			return true;
		}
		if (Search(key)) {
			_latch.unlock();
			return false;
		}
		int level = RandomLevel();
		int levelCount = _levelCount;
		_levelCount = std::max(_levelCount, level);
		NodePtr q = nullptr;
		for (int i = level; i > levelCount; i--) {
			NodePtr new_head = new Node(_head);
			_head->_latch.WriteLock();
			new_head->_latch.WriteLock();
			new_head->Delete();
			NodePtr node = new Node(key, value);
			new_head->_next = node;
			new_head->_down = _head;
			node->_prev = new_head;
			node->_down = q;
			q = node;
			_head->_latch.WriteUnlock();
			_head = new_head;
			new_head->_latch.WriteUnlock();
		}
		q = nullptr;
		NodePtr node = _head;
		while (level > levelCount) {
			q = node->_next;
			node = node->_down;
			level--;
		}
		while (level < levelCount) {
			while (node->_next && node->_next->GetKey() < key) {
				node = node->_next;
			}
			node = node->_down;
			levelCount--;
		}
		while (node) {
			while (node->_next && node->_next->GetKey() < key) {
				node = node->_next;
			}
			NodePtr p = new Node(key, value);
			p->_next = node->_next;
			p->_prev = node;
			if (p->_next) {
				p->_next->_prev = p;
			}
			node->_next = p;
			if (q) {
				q->_down = p;
			}
			q = p;
			node = node->_down;
		}
		_latch.unlock();
		return true;
	}

	bool Delete(const_K_ref key) {	
		NodePtr node = Search(key);
		if (!node) return false;

		node->_latch.WriteLock();
		node->Delete();
		node->_latch.WriteUnlock();
		return true;
	}
	NodePtr Search(const_K_ref key) {
		if (!_head) {
			return nullptr;
		}
		NodePtr node = _head;
		node->_latch.ReadLock();
		while (node && node->_down) {
			while (node->_next && key >= node->_next->GetKey()) {
				node->_latch.ReadUnlock();
				node = node->_next;
				node->_latch.ReadLock();
			}
			node->_latch.ReadUnlock();
			node = node->_down;
			if(node) node->_latch.ReadLock();
		}
		while (node && node->GetKey() < key) {
			node->_latch.ReadUnlock();
			node = node->_next;
			if(node) node->_latch.ReadLock();
		}
		if (!node) return nullptr;
		node->_latch.ReadUnlock();
		return node->GetKey() == key && node->Exist() ? node : nullptr;
	}
	bool Revise(const_K_ref key, const_V_ref value) {
		NodePtr node = Search(key);
		if (!node) {
			return false;
		}
		if (node->_latch.WriteLock()) {
			node->_value = value;
			node->_latch.WriteUnlock();
		}
		return true;
	}
	
	std::vector<V> GetAllValue() {
		std::vector<V> res;
		NodePtr node = _head;
		if (!node) {
			return res;
		}
		while (node->_down) {
			node = node->_down;
		}
		while (node) {
			res.push_back(node->GetValue());
			node = node->_next;
		}
		return res;
	}
	std::vector<std::pair<K, V>> GetAll() {
		std::vector<std::pair<K, V>> res;
		NodePtr node = _head;
		if (!node) {
			return res;
		}
		while (node->_down) {
			node = node->_down;
		}
		while (node) {
			if (node->Exist()) {
				res.push_back({ node->GetKey(),node->GetValue() });
			}
			node = node->_next;
		}
		return res;
	}
	void ShowAll() {
		NodePtr node = _head;
		while (node) {
			NodePtr p = node;
			while (p) {
				std::cout << "(" << p->GetKey() << "," << p->GetValue() << "," <<p->Exist() << ") -> ";
				p = p->_next;
			}
			std::cout << std::endl;
			node = node->_down;
		}
	}
	std::vector<V> RangeIn(const_K_ref low, const_K_ref high) {
		NodePtr start = GetGreater(low);
		std::vector<V> res;
		while (start && start->GetKey() < high) {
			res.push_back(start->GetValue());
			start = start->_next;
		}
		return res;
	}
	void ShowStruct() {
		NodePtr node = _head;
		while (node) {
			NodePtr p = node;
			while (p) {
				std::cout << "(" << p->GetKey() << ") -> ";
				p = p->_next;
			}
			std::cout << std::endl;
			node = node->_down;
		}
	}
private:
	inline int RandomLevel() {
		int res = 0;
		while (rand() % 100 < POSSIBILITY && res < MAXLEVEL) {
			res++;
		}
		return res;
	}
	inline NodePtr GetGreater(const_K_ref key) {
		if (!_head) return nullptr;
		NodePtr node = _head;
		while (node->_down) {
			while (node->_next && key > node->_next->GetKey()) {
				node = node->_next;
			}
			node = node->_down;
		}
		while (node && node->GetKey() < key) {
			node = node->_next;
		}
		return node;
	}
	int _levelCount = 0;
	NodePtr _head = nullptr;
	int POSSIBILITY = 50;
	int MAXLEVEL = 32;
	std::mutex _latch;
};