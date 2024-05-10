#include"ReadWriteLock.h"
ReadWriteLock::ReadWriteLock() {

}
ReadWriteLock::~ReadWriteLock() {

}
bool ReadWriteLock::ReadLock() {
	std::unique_lock<std::mutex> latch(_latch);
	_read_latch.wait(latch, [this]() {return !_is_change; });
	_reader_count++;
	return true;
}
void ReadWriteLock::ReadUnlock() {
	std::unique_lock<std::mutex> latch(_latch);
	_reader_count--;
	if (_reader_count == 0) {
		_write_latch.notify_one();
	}
}
bool ReadWriteLock::WriteLock() {
	std::unique_lock<std::mutex> latch(_latch);
	_writer_count++;
	_write_latch.wait(latch, [this]() {return !_is_change && !_reader_count; });
	_is_change = true;
	return true;
}
void ReadWriteLock::WriteUnlock() {
	std::unique_lock<std::mutex> latch(_latch);
	_writer_count--;
	_is_change = false;
	_write_latch.notify_one();
	_read_latch.notify_all();
}
