#ifndef _SENDER_HPP__
#define _SENDER_HPP__

#include <thread>

#include "channel.hpp"
#include "file_pdu_stream.hpp"
#include "slide_window.hpp"
#include "config.hpp"

class Sender {
	uint64_t timeout_;
	uint16_t seq_;
	Channel channel_;
	FilePDUStream file_stream_;
	std::thread thread_;
public:
	Sender(in_addr_t ip, uint16_t port, const char *file_path,
		uint16_t init_seq_no = Config::init_seqno, uint64_t timeout = Config::timeout);

	void run();
	void wait();

	Sender(const Sender &) = delete;
	Sender &operator=(const Sender &) = delete;
};

#endif 