#pragma once
#pragma once
#include"Parallel_SkipList.h"
#include"DiskManager.h"
template<typename K, typename V, typename DataStruct = Parallel_SkipList<K, V>, typename DiskManager = DiskManager<K, V, DataStruct>>
class Parallel_DataBase {
	typedef DataStruct		DB;
	typedef const K&		const_K_ref;
	typedef const V&		const_V_ref;
public:
	Parallel_DataBase(const std::string file_path):_file_path(file_path) {
		for (int i = 0; i < _db_size; i++) {
			_db.push_back(DB());
		}
		_disk_manager = DiskManager(file_path);
	}
	Parallel_DataBase(const std::string file_path, std::vector<std::pair<K, V>>& data):_file_path(file_path) {
		for (int i = 0; i < _db_size; i++) {
			_db.push_back(DB(data));
		}
		_disk_manager = DiskManager(file_path);
	}
	bool Start() {
		for (int i = 0; i < _db_size; i++) {
			std::string str = _file_path + "_" + std::to_string(i);
			_disk_manager.ReadFromDisk(str, _db[i]);
		}
		return true;
	}
	bool Close() {
		for (int i = 0; i < _db_size; i++) {
			std::string str = _file_path + "_" + std::to_string(i);
			std::vector<std::pair<K, V>> data = _db[i].GetAll();
			_disk_manager.WriteToDisk(str, data);
		}
		return true;
	}
	bool Insert(const_K_ref key, const_V_ref value) {
		return _db[Hash(key)].Insert(key, value);
	}

	std::vector<K> res;
	bool Delete(const_K_ref key) {
		//std::cout << key << std::endl;
		//_db.ShowStruct();
		if (_db[Hash(key)].Delete(key)) {
			res.push_back(key);
		}
		//return _db[Hash(key)].Delete(key);
		return 1;
	}
	V Search(const_K_ref key) {
		if (!_db[Hash(key)].Search(key)) {
			return {};
		}
		return _db[Hash(key)].Search(key)->GetValue();
	}
	void RangeIn(const_K_ref low, const_K_ref high) {
		if (low > high) {
			return;
		}
		std::vector<V> res;
		for (auto& db : _db) {
			std::vector<V> values = db.RangeIn(low, high);
			res.insert(res.end(), values.begin(), values.end());
		}
		// sort(res.begin(), res.end());
		for (auto& value : res) {
			std::cout << value << " -> ";
		}
		std::cout << std::endl;
	}
	bool Revise(const_K_ref key, const_V_ref value) {
		return _db[Hash(key)].Revise(key, value);
	}
	void ShowAll() {
		for(auto& db:_db)
			db.ShowAll();
	}
private:
	inline int Hash(K key) {
		return key % _db_size;
	}
	const int _db_size = 10;
	std::vector<DB> _db;
	std::string _file_path;
	DiskManager _disk_manager;
};