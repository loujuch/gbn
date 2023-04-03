#ifndef _CONFIG_HPP__
#define _CONFIG_HPP__

#include "log.hpp"

#include <stdint.h>

/**
 * 参数配置
*/
class Config {
public:
	static uint16_t udp_port;
	static uint16_t data_size;
	static uint16_t timeout;
	static uint16_t sw_size;
	static uint8_t error_rate;
	static uint8_t lost_rate;
	static uint8_t init_seqno;
	static std::string file_root;

	static bool paser_config_file(const char *file_name);

	Config() = delete;
	Config(const Config &) = delete;
	Config &operator=(const Config &) = delete;
};

#endif