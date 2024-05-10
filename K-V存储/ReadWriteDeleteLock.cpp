#include"ReadWriteDeleteLock.h"
ReadWriteDeleteLock::ReadWriteDeleteLock() {

}
ReadWriteDeleteLock::~ReadWriteDeleteLock() {
	DeleteUnlock();
}
bool ReadWriteDeleteLock::ReadLock() {
	if (_is_erase) return false;
	std::unique_lock<std::mutex> latch(_latch);
	_read_latch.wait(latch, [this]() {return !_is_change; });
	_reader_count++;
	return true;
}
void ReadWriteDeleteLock::ReadUnlock() {
	std::unique_lock<std::mutex> latch(_latch);
	_reader_count--;
	if (_reader_count == 0) {
		_write_latch.notify_one();
	}
}
bool ReadWriteDeleteLock::WriteLock() {
	if (_is_erase) return false;
	std::unique_lock<std::mutex> latch(_latch);
	_writer_count++;
	_write_latch.wait(latch, [this]() {return !_is_change && !_reader_count; });
	_is_change = true;
	return true;
}
void ReadWriteDeleteLock::WriteUnlock() {
	std::unique_lock<std::mutex> latch(_latch);
	_writer_count--;
	_is_change = false;
	_write_latch.notify_one();
	_read_latch.notify_all();
}
void ReadWriteDeleteLock::DeleteLock() {
	if (_is_erase) return;
	std::unique_lock<std::mutex> latch(_latch);
	_delete_latch.wait(latch, [this]() {return !_reader_count && !_writer_count; });
	_is_erase = true;
	_delete.lock();
}
void ReadWriteDeleteLock::DeleteUnlock() {
	_delete.unlock();
}