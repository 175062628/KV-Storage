#pragma once
#include<mutex>
#include <condition_variable>
class ReadWriteDeleteLock {
public:
	ReadWriteDeleteLock();
	~ReadWriteDeleteLock();
	bool ReadLock();
	void ReadUnlock();
	bool WriteLock();
	void WriteUnlock();
	void DeleteLock();
	void DeleteUnlock();
private:
	std::mutex _latch, _delete;
	std::condition_variable _read_latch, _write_latch, _delete_latch;
	int _reader_count = 0;
	int _writer_count = 0;
	bool _is_change = false;
	bool _is_erase = false;
};