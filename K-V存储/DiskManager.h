#pragma once
#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include"SkipList.h"
template<typename K, typename V, typename DataStruct>
class DiskManager {
	typedef DataStruct		DB;
public:
	DiskManager() {

	}
	DiskManager(std::string file_path) :_file_path(file_path) {

	}
	bool WriteToDisk(std::vector<std::pair<K, V>>& data) {
		std::ofstream file(_file_path + ".txt", std::ios::out | std::ios::trunc);
		if (!file.is_open()) {
			return false;
		}
		for (int i = 1; i < data.size(); i++) {
			file << data[i].first << "\n" << data[i].second << "\n";
		}
		file.close();
		return true;
	}
	bool WriteToDisk(std::string& file_path, std::vector<std::pair<K, V>>& data) {
		std::ofstream file(file_path + ".txt", std::ios::out | std::ios::trunc);
		if (!file.is_open()) {
			return false;
		}
		for (auto& [key, value] : data) {
			file << key << "\n" << value << "\n";
		}
		file.close();
		return true;
	}
	bool ReadFromDisk(DB& db) {
		std::ifstream file(_file_path + ".txt");
		if (!file.is_open()) return false;
		std::string _key,_value;
		while (std::getline(file, _key)) {
			K key = String2K(_key);
			std::getline(file, _value);
			V value = String2V(_value);
			db.Insert(key, value);
		}
		file.close();
		return true;
	}
	bool ReadFromDisk(std::string& file_path, DB& db) {
		std::ifstream file(file_path + ".txt");
		if (!file.is_open()) return false;
		std::string _key, _value;
		while (std::getline(file, _key)) {
			K key = String2K(_key);
			std::getline(file, _value);
			V value = String2V(_value);
			db.Insert(key, value);
		}
		file.close();
		return true;
	}
private:
	inline K String2K(const std::string& str) {
		K res = 0, flag = 1;
		for (auto& c : str) {
			if (c == '-') {
				flag = -1;
				continue;
			}
			res = 10 * res + (c - '0');
		}
		return flag * res;
	}
	inline V String2V(std::string& str) {
		return str;
	}
	std::string _file_path;
};