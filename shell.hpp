#ifndef _SHELL_HPP__
#define _SHELL_HPP__

#include <memory>

#include "receiver.hpp"

/**
 * 提供简易的交互界面和控制方法
*/
class Shell {
	uint64_t size_;
	std::shared_ptr<char> buffer_;
	Receiver receiver;
public:
	Shell(uint64_t buffer_size = 1024);
	int exec();

	Shell(const Shell &) = delete;
	Shell &operator=(const Shell &) = delete;
};

#endif