#ifndef _LOG_HPP__
#define _LOG_HPP__

#include <string>
#include <mutex>

/**
 * 日志类，用于写入日志
*/
class SendLog {
	static uint64_t no_;
	static uint16_t ack_;
	static uint16_t seq_;
	static int file_;
	static std::mutex mutex_;
public:
	static void new_log();
	static void recv_ack(uint16_t seq);
	static void new_seq(uint16_t seq);
	static void record(bool is_re);
	static void close_log();
};

class RecvLog {
	static uint64_t no_;
	static uint16_t hope_;
	static uint16_t seq_;
	static int file_;
	static std::mutex mutex_;
public:
	static void new_log();
	static void hope_recv(uint16_t seq);
	static void now_recv(uint16_t seq);
	static void record(bool is_error);
	static void close_log();
};

#endif // _LOG_HPP__