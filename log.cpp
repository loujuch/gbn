#include "log.hpp"

#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>

uint64_t SendLog::no_ = 0;
uint16_t SendLog::ack_ = 0;
uint16_t SendLog::seq_ = 0;
int SendLog::file_ = -1;
std::mutex SendLog::mutex_;

void SendLog::new_log() {
	if(file_ != -1) {
		close(file_);
	}
	char buffer[1024];
	timespec tmp;
	clock_settime(CLOCK_REALTIME, &tmp);
	snprintf(buffer, 1024, "IN_%ld%ld.log", tmp.tv_sec, tmp.tv_nsec);
	file_ = open(buffer, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	no_ = 1;
	ack_ = 0;
	seq_ = 0;
}

void SendLog::recv_ack(uint16_t seq) {
	ack_ = seq;
}

void SendLog::new_seq(uint16_t seq) {
	seq_ = seq;
}

void SendLog::record(bool is_re) {
	char buffer[1024];
	snprintf(buffer, 1024, "%lu, pdu_to_send=%u, status=%s, ackedNo=%u\n", no_, seq_, is_re ? "TO" : "New", ack_);
	int n = strlen(buffer);
	mutex_.lock();
	write(file_, buffer, n);
	mutex_.unlock();
	++no_;
}

void SendLog::close_log() {
	if(file_ == -1) {
		return;
	}
	close(file_);
	file_ = -1;
}


uint64_t RecvLog::no_ = 0;
uint16_t RecvLog::hope_ = 0;
uint16_t RecvLog::seq_ = 0;
int RecvLog::file_ = -1;
std::mutex RecvLog::mutex_;

void RecvLog::new_log() {
	if(file_ != -1) {
		close(file_);
	}
	char buffer[1024];
	timespec tmp;
	clock_settime(CLOCK_REALTIME, &tmp);
	snprintf(buffer, 1024, "OUT_%ld%ld.log", tmp.tv_sec, tmp.tv_nsec);
	file_ = open(buffer, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	no_ = 1;
	hope_ = 0;
	seq_ = 0;
}

void RecvLog::hope_recv(uint16_t seq) {
	hope_ = seq;
}

void RecvLog::now_recv(uint16_t seq) {
	seq_ = seq;
}

void RecvLog::record(bool is_error) {
	char buffer[1024];
	snprintf(buffer, 1024, "%lu, pdu_exp=%u, pdu_recv=%u, status=%s\n",
		no_, hope_, seq_, is_error ? "DataErr" : (hope_ == seq_ ? "OK" : "NoErr"));
	int n = strlen(buffer);
	mutex_.lock();
	write(file_, buffer, n);
	mutex_.unlock();
	++no_;
}

void RecvLog::close_log() {
	if(file_ == -1) {
		return;
	}
	close(file_);
	file_ = -1;
}