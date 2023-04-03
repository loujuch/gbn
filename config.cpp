#include "config.hpp"

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>

uint16_t Config::udp_port = 41453;
uint16_t Config::data_size = 1024;
uint16_t Config::timeout = 1000;
uint16_t Config::sw_size = 4;
uint8_t Config::error_rate = 0;
uint8_t Config::lost_rate = 0;
uint8_t Config::init_seqno = 0;
std::string Config::file_root = "./";

/* config文件的格式应为：^<key>: <value>$ */
bool Config::paser_config_file(const char *file) {
	char *line = nullptr;
	size_t size;
	FILE *config_file = fopen(file, "r");
	assert(config_file != nullptr);

	while(getline(&line, &size, config_file) != -1) {
		assert(line != nullptr);
		char *p = line;
		while(*p != ':' && *p != '\0') {
			++p;
		}
		if(*p == '\0') {
			continue;
		}
		*p = '\0';
		++p;
		while(*p == ' ') {
			++p;
		}
		if(*p == '\0') {
			continue;
		}
		if(strcmp(line, "UDPPort") == 0) {
			udp_port = atoi(p);
			assert(udp_port != 0);
		} else if(strcmp(line, "DataSize") == 0) {
			data_size = atoi(p);
			assert(data_size != 0);
		} else if(strcmp(line, "ErrorRate") == 0) {
			error_rate = atoi(p);
		} else if(strcmp(line, "LostRate") == 0) {
			lost_rate = atoi(p);
		} else if(strcmp(line, "SWSize") == 0) {
			sw_size = atoi(p);
			assert(sw_size != 0);
		} else if(strcmp(line, "InitSeqNo") == 0) {
			init_seqno = atoi(p);
		} else if(strcmp(line, "Timeout") == 0) {
			timeout = atoi(p);
			assert(timeout != 0);
		} else if(strcmp(line, "FileRoot") == 0) {
			file_root = p;
			while(file_root.back() == '\n') {
				file_root.pop_back();
			}
			if(file_root.back() != '/') {
				file_root.push_back('/');
			}
		} else {
			continue;
		}
	}

	if(line != nullptr) {
		free(line);
	}
	fclose(config_file);

	return true;
}