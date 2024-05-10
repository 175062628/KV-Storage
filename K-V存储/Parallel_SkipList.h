#pragma once
#include<vector>
#include<iostream>
#include<mutex>
#include"Parallel_SkipListNode.h"
template<typename K, typename V>
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

	/*
		互斥锁版
	*/
	bool Insert(const_K_ref key, const_V_ref value) {
		_latch.lock();
		// 插入第一个节点
		if (!_head) {
			_head = new Node(key, value);
			_latch.unlock();
			return true;
		}
		// 重复插入
		//if (Search(key)) return false;
		bool exist = false;
		{
			NodePtr node = _head;
			while (node && node->_down) {
				while (node->_next && key >= node->_next->GetKey()) {
					node = node->_next;
				}
				node = node->_down;
			}
			while (node && node->GetKey() < key) {
				node = node->_next;
			}
			if (node && node->GetKey() == key) {
				exist = true;
			}
		}
		if (exist) {
			_latch.unlock();
			return false;
		}
		// 插入头结点
		if (key < _head->GetKey()) {
			NodePtr new_head = new Node(key, value), node = _head;
			_head = new_head;
			while (node) {
				new_head->_next = node;
				node->_prev = new_head;
				node = node->_down;
				if (!node) break;
				new_head->_down = new Node(key, value);
				new_head = new_head->_down;
			}
			_latch.unlock();
			return true;
		}
		// 插入非头节点
		int level = RandomLevel();
		int levelCount = _levelCount;
		_levelCount = std::max(_levelCount, level);
		NodePtr q = nullptr;
		for (int i = level; i > levelCount; i--) {
			NodePtr new_head = new Node(_head);
			NodePtr node = new Node(key, value);
			new_head->_next = node;
			new_head->_down = _head;
			node->_prev = new_head;
			node->_down = q;
			q = node;
			_head = new_head;
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
	/*bool Delete(const_K_ref key) {
		_latch.lock();
		// 空表特判
		if (!_head) {
			_latch.unlock();
			return false;
		}
		NodePtr node = _head;
		// 删除头结点
		if (node && node->GetKey() == key) {
			std::vector<NodePtr> next_nodes;
			while (node) {
				NodePtr cur = node;
				// 自旋锁
				while (node->_latch.Locked());
				if (node->_next) next_nodes.push_back(node->_next);
				node = node->_down;
				delete cur;
			}
			_levelCount = next_nodes.size();
			if (next_nodes.empty()) {
				_head = nullptr;
				_latch.unlock();
				return true;
			}
			NodePtr new_head = new Node(next_nodes.back());
			_head = new_head;
			for (auto& next_node : next_nodes) {
				if (next_node->GetKey() == new_head->GetKey()) {
					if (next_node->_next) next_node->_next->_prev = new_head;
					new_head->_next = next_node->_next;
					new_head->_down = next_node->_down;
					while (next_node->_latch.Locked());
					delete next_node;
					break;
				}
				new_head->_next = next_node;
				next_node->_prev = new_head;
				new_head->_down = new Node(next_nodes.back());
				new_head = new_head->_down;
			}
			_latch.unlock();
			return true;
		}
		// 删除非头节点
		while (node && node->_down) {
			while (node->_next && key >= node->_next->GetKey()) {
				node = node->_next;
			}
			if (node->GetKey() == key) break;
			node = node->_down;
		}
		while (node && node->GetKey() < key) {
			node = node->_next;
		}
		if (!node || node->GetKey() != key) {
			_latch.unlock();
			return false;
		}
		while (node) {
			NodePtr cur = node, p = cur->_prev, q = cur->_next;
			node = node->_down;
			if (!cur->_next && !cur->_prev->_prev) {
				_head = p->_down;
				_levelCount--;
				while (p->_latch.Locked());
				delete p;
			}
			else {
				p->_next = q;
				if (q) q->_prev = p;
			}
			while (cur->_latch.Locked());
			delete cur;
		}
		_latch.unlock();
		return true;
	}
	NodePtr Search(const_K_ref key) {
		_latch.lock();
		if (!_head) {
			_latch.unlock();
			return nullptr;
		}
		NodePtr node = _head;
		while (node && node->_down) {
			while (node->_next && key >= node->_next->GetKey()) {
				node = node->_next;
			}
			node = node->_down;
		}
		while (node && node->GetKey() < key) {
			node = node->_next;
		}
		if (!node) {
			_latch.unlock();
			return nullptr;
		}
		_latch.unlock();
		return node->GetKey() == key ? node : nullptr;
	}
	bool Revise(const_K_ref key, const_V_ref value) {
		_latch.lock();
		if (!_head) {
			_latch.unlock();
			return false;
		}
		NodePtr node = _head;
		while (node && node->_down) {
			while (node->_next && key >= node->_next->GetKey()) {
				node = node->_next;
			}
			node = node->_down;
		}
		while (node && node->GetKey() < key) {
			node = node->_next;
		}
		if (!node || node->GetKey() != key) {
			_latch.unlock();
			return false;
		}
		node->_value = value;
		_latch.unlock();
		return true;
	}*/


	//bool Insert(const_K_ref key, const_V_ref value) {
	//	// 插入第一个节点
	//	if (!_head) {
	//		_head = new Node(key, value);
	//		return true;
	//	}
	//	// 重复插入
	//	if (Search(key)) return false;
	//	if (exist) return false;
	//	// 插入头结点
	//	if (key < _head->GetKey()) {
	//		NodePtr new_head = new Node(key, value), node = _head;
	//		new_head->_latch.WriteLock();
	//		node->_latch.WriteLock();
	//		_head = new_head;
	//		while (node) {
	//			new_head->_next = node;
	//			node->_prev = new_head;
	//			node->_latch.WriteUnlock();
	//			node = node->_down;
	//			if (!node) {
	//				break;
	//			}
	//			node->_latch.WriteLock();
	//			new_head->_down = new Node(key, value);
	//			new_head->_latch.WriteUnlock();
	//			new_head = new_head->_down;
	//			new_head->_latch.WriteLock();
	//		}
	//		new_head->_latch.WriteUnlock();
	//		return true;
	//	}
	//	// 插入非头节点
	//	int level = RandomLevel();
	//	int levelCount = _levelCount;
	//	_levelCount = std::max(_levelCount, level);
	//	NodePtr q = nullptr;
	//	for (int i = level; i > levelCount; i--) {
	//		NodePtr new_head = new Node(_head);
	//		new_head->_latch.WriteLock();
	//		NodePtr node = new Node(key, value);
	//		new_head->_next = node;
	//		new_head->_down = _head;
	//		node->_prev = new_head;
	//		node->_down = q;
	//		q = node;
	//		_head = new_head;
	//		new_head->_latch.WriteUnlock();
	//	}
	//	q = nullptr;
	//	NodePtr node = _head;
	//	while (level > levelCount) {
	//		q = node->_next;
	//		node = node->_down;
	//		level--;
	//	}
	//	while (level < levelCount) {
	//		while (node->_next && node->_next->GetKey() < key) {
	//			node = node->_next;
	//		}
	//		node = node->_down;
	//		levelCount--;
	//	}
	//	while (node) {
	//		while (node->_next && node->_next->GetKey() < key) {
	//			node = node->_next;
	//		}
	//		node->_latch.WriteLock();
	//		NodePtr p = new Node(key, value);
	//		p->_latch.WriteLock();
	//		p->_next = node->_next;
	//		p->_prev = node;
	//		if (p->_next) {
	//			p->_next->_prev = p;
	//		}
	//		node->_next = p;
	//		if (q) {
	//			q->_down = p;
	//		}
	//		q = p;
	//		p->_latch.WriteUnlock();
	//		node->_latch.WriteUnlock();
	//		node = node->_down;
	//	}
	//	return true;
	//}
	bool Delete(const_K_ref key) {	
		// 空表特判
		if (!_head) {
			return false;
		}
		NodePtr node = _head;
		// 删除头结点
		if (node && node->GetKey() == key) {
			std::vector<NodePtr> next_nodes;
			_head = new Node();
			_head->_latch.WriteLock();
			while (node) {
				node->_latch.DeleteLock();
				NodePtr cur = node;
				if (node->_next) {
					node->_next->_latch.WriteLock();
					next_nodes.push_back(node->_next);
				}
				node = node->_down;
				delete cur;
			}
			_levelCount = next_nodes.size();
			if (next_nodes.empty()) {
				_head = nullptr;
				return true;
			}
			NodePtr new_head = new Node(next_nodes.back());
			_head->_down = new_head;
			new_head->_latch.WriteLock();
			NodePtr up = nullptr;
			bool special = 0;
			for (auto it = next_nodes.begin(); it != next_nodes.end(); it++) {
				if ((*it)->GetKey() == next_nodes.back()->GetKey()) {
					if (new_head) {
						if (!up) {
							_head = *it;
							special = true;
						}
						else up->_down = *it;
						new_head->_latch.WriteUnlock();
						new_head->_latch.DeleteLock();
						delete new_head;
						new_head = nullptr;
					}
					(*it)->_prev = nullptr;
					(*it)->_latch.WriteUnlock();
					continue;
				}
				new_head->_next = *it;
				(*it)->_prev = new_head;
				(*it)->_latch.WriteUnlock();
				new_head->_down = new Node(next_nodes.back());
				new_head->_latch.WriteUnlock();
				up = new_head;
				new_head = new_head->_down;
				new_head->_latch.WriteLock();
			}
			if (!special) {
				_head->_latch.WriteUnlock();
				_head = _head->_down;
			}
			return true;
		}
		// 删除非头节点
		node->_latch.ReadLock();
		while (node->_down) {
			while (node->_next && key >= node->_next->GetKey()) {
				node->_latch.ReadUnlock();
				node = node->_next;
				node->_latch.ReadLock();
			}
			if (node->GetKey() == key) {
				node->_latch.ReadUnlock();
				node->_latch.DeleteLock();
				break;
			}
			node->_latch.ReadUnlock();
			node = node->_down;
			node->_latch.ReadLock();
		}

		while (node && node->GetKey() < key) {
			node->_latch.ReadUnlock();
			node = node->_next;
			if(node) node->_latch.ReadLock();
		}
		if (!node || node->GetKey() != key) {
			if (node && node->GetKey() != key) {
				node->_latch.ReadUnlock();
			}
			return false;
		}
		node->_latch.ReadUnlock();
		while (node) {
			NodePtr cur = node, p = cur->_prev, q = cur->_next;
			cur->_latch.DeleteLock();
			node = node->_down;
			//if (node) node->_latch.DeleteLock();
			if (p->_latch.WriteLock()) {
				p->_next = q;
				p->_latch.WriteUnlock();
			}
			if (q && q->_latch.WriteLock()) {
				q->_prev = p;
				q->_latch.WriteUnlock();
			}
			delete cur;
		}
		return true;
	}
	NodePtr Search(const_K_ref key) {
		if (!_head) {
			return nullptr;
		}
		NodePtr node = _head;
		node->_latch.ReadLock();
		while (node->_down) {
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
		return node->GetKey() == key ? node : nullptr;
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
			res.push_back({ node->GetKey(),node->GetValue() });
			node = node->_next;
		}
		return res;
	}
	void ShowAll() {
		NodePtr node = _head;
		while (node) {
			NodePtr p = node;
			while (p) {
				std::cout << "(" << p->GetKey() << "," << p->GetValue() << ") -> ";
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