#pragma once
#include<mutex>
#include <condition_variable>
class ReadWriteLock {
public:
	ReadWriteLock();
	~ReadWriteLock();
	bool ReadLock();
	void ReadUnlock();
	bool WriteLock();
	void WriteUnlock();
private:
	std::mutex _latch;
	std::condition_variable _read_latch, _write_latch;
	int _reader_count = 0;
	int _writer_count = 0;
	bool _is_change = false;
};