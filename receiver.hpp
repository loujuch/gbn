#ifndef _RECEIVER_HPP__
#define _RECEIVER_HPP__

#include <thread>

#include "channel.hpp"
#include "config.hpp"

class Receiver {
	std::thread thread_;
	Channel channel_;
public:
	Receiver(uint16_t port = Config::udp_port,
		uint8_t error_rate = Config::error_rate,
		uint8_t lost_rate = Config::lost_rate);

	void run();
};


#endif