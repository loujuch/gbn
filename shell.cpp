#include "shell.hpp"

#include <iostream>

#include "config.hpp"
#include "sender.hpp"

#include <assert.h>
#include <string.h>

enum OPT {
	UNKNOWN = 0,
	CLOSE,
	SEND
};

enum OPT get_type(const char *p) {
	if(strcmp(p, "close") == 0) {
		return CLOSE;
	}
	if(strcmp(p, "send") == 0) {
		return SEND;
	}
	return UNKNOWN;
}

int split_line(char *word[], uint64_t size, char *s) {
	assert(s != nullptr);
	int len = 0;
	while(*s != '\0' && len < size) {
		while(*s == ' ' || *s == '\t') {
			++s;
		}
		if(*s == '\0') {
			break;
		}
		word[len++] = s;
		while(*s != '\0' && *s != '\t' && *s != ' ') {
			++s;
		}
		if(*s != '\0') {
			*s = '\0';
			++s;
		}
	}
	return len;
}

bool send_file(const char *ip_str, const char *port_str, const char *file_name) {
	in_addr_t ip = inet_addr(ip_str);
	if(ip == INADDR_NONE) {
		return false;
	}
	uint16_t port = atoi(port_str);
	if(port == 0) {
		return false;
	}
	Sender sender(ip, port, file_name);
	sender.run();
	sender.wait();
	return true;
}

Shell::Shell(uint64_t size):
	size_(size),
	buffer_(new char[size]) {
	receiver.run();
}

int Shell::exec() {
	do {
		std::cout << Config::udp_port << "> ";
		std::cin.getline(buffer_.get(), size_);
		char *word[8];
		int len = split_line(word, 8, buffer_.get());
		if(len != 0) {
			switch(get_type(word[0])) {
				case CLOSE:
					return 0;
				case SEND:
					if(!send_file(word[1], word[2], word[3])) {
						std::cout << word[0] << ' ' << word[1] << ' ' << word[2]
							<< ' ' << word[3] << ": error!" << std::endl;
					}
					break;
				case UNKNOWN:
					std::cout << word[0] << ": unknown command\n";
					break;
			}
		}
	} while(!std::cin.eof());
	std::cout << '\n';
	return 0;
}