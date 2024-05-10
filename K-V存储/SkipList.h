#pragma once
#include<vector>
#include<iostream>
#include"SkipListNode.h"
template<typename K,typename V>
class SkipList {
public:
	typedef SkipListNode<K, V>		Node;
	typedef SkipListNode<K, V>*		NodePtr;
	typedef const K&				const_K_ref;
	typedef const V&				const_V_ref;
public:
	SkipList() {

	}
	SkipList(std::vector<std::pair<K,V>>& data) {
		for (auto& [key, value] : data) {
			Insert(key, value);
		}
	}
	SkipList(const SkipList& skiplist) {
		this->_levelCount = skiplist._levelCount;
		this->_head = skiplist._head;
		this->POSSIBILITY = skiplist.POSSIBILITY;
		this->MAXLEVEL = skiplist.MAXLEVEL;
	}
	SkipList& operator=(const SkipList& skiplist) {
		if (this != &skiplist) {
			this->_levelCount = skiplist._levelCount;
			this->_head = skiplist._head;
			this->POSSIBILITY = skiplist.POSSIBILITY;
			this->MAXLEVEL = skiplist.MAXLEVEL;
		}
		return *this;
	}
	bool Insert(const_K_ref key, const_V_ref value) {
		
		// 插入第一个节点
		if (!_head) {
			_head = new Node(key, value);
			
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
		if (exist) return false;

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
		
		return true;
	}
	bool Delete(const_K_ref key) {
		
		// 空表特判
		if (!_head) {
			
			return false;
		}
		NodePtr node = _head;
		// 删除头结点
		if (node->GetKey() == key) {
			std::vector<NodePtr> next_nodes;
			while (node) {
				NodePtr cur = node;
				if(node->_next) next_nodes.push_back(node->_next);
				node = node->_down;
				delete cur;
			}
			_levelCount = (int)next_nodes.size();
			if (next_nodes.empty()) {
				_head = nullptr;
				
				return true;
			}
			NodePtr new_head = new Node(next_nodes.back());
			_head = new_head;
			for (auto& next_node : next_nodes) {
				if (next_node->GetKey() == new_head->GetKey()) {
					if(next_node->_next) next_node->_next->_prev = new_head;
					new_head->_next = next_node->_next;
					new_head->_down = next_node->_down;
					delete next_node;
					break;
				}
				new_head->_next = next_node;
				next_node->_prev = new_head;
				new_head->_down = new Node(next_nodes.back());
				new_head = new_head->_down;
			}
			
			return true;
		}
		// 删除非头节点
		while (node->_down) {
			while (node->_next && key >= node->_next->GetKey()) {
				node = node->_next;
			}
			if (node->GetKey() == key) break;
			node = node->_down;
		}
		while (node && node->GetKey() < key) {
			node = node->_next;
		}
		if (!node || node->GetKey() != key) return false;
		while (node) {
			NodePtr cur = node, p = cur->_prev, q = cur->_next;
			node = node->_down;
			if (!cur->_next && !cur->_prev->_prev) {
				_head = p->_down;
				_levelCount--;
				delete p;
			}
			else {
				p->_next = q;
				if (q) q->_prev = p;
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
		while (node && node->_down) {
			while (node->_next && key >= node->_next->GetKey()) {
				node = node->_next;
			}
			node = node->_down;
		}
		while (node && node->GetKey() < key) {
			node = node->_next;
		}
		if (!node) return nullptr;
		return node->GetKey() == key ? node : nullptr;
	}
	bool Revise(const_K_ref key, const_V_ref value) {
		
		NodePtr node = Search(key);
		if (!node) {
			
			return false;
		}
		node->_value = value;
		
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
};