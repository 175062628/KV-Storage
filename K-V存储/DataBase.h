#pragma once
#include"SkipList.h"
#include"DiskManager.h"
template<typename K,typename V, typename DataStruct = SkipList<K, V>, typename DiskManager = DiskManager<K, V, DataStruct>>
class DataBase {
	typedef DataStruct		DB;
	typedef const K&		const_K_ref;
	typedef const V&		const_V_ref;
public:
	DataBase(const std::string file_path) {
		_db = DB();
		_disk_manager = DiskManager(file_path);
	}
	DataBase(const std::string file_path, std::vector<std::pair<K, V>>& data) {
		_db = DB(data);
		_disk_manager = DiskManager(file_path);
	}
	bool Start() {
		return _disk_manager.ReadFromDisk(_db);
	}
	bool Close() {
		std::vector<std::pair<K, V>> data = _db.GetAll();
		return _disk_manager.WriteToDisk(data);
	}
	bool Insert(const_K_ref key, const_V_ref value) {
		return _db.Insert(key, value);
	}
	bool Delete(const_K_ref key) {
		return _db.Delete(key);
	}
	V Search(const_K_ref key) {
		if (!_db.Search(key)) {
			return {};
		}
		return _db.Search(key)->GetValue();
	}
	void RangeIn(const_K_ref low, const_K_ref high) {
		if (low > high) {
			return;
		}
		std::vector<V> values = _db.RangeIn(low, high);
		for (auto& value : values) {
			std::cout << value << " -> ";
		}
		std::cout << std::endl;
	}
	bool Revise(const_K_ref key, const_V_ref value) {
		return _db.Revise(key, value);
	}
	void ShowAll() {
		_db.ShowAll();
	}
private:
	DB _db;
	DiskManager _disk_manager;
};