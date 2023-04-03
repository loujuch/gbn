#include "config.hpp"
#include "shell.hpp"
// #include "receiver.hpp"
#include "file_pdu_stream.hpp"

#include <stdio.h>
#include <assert.h>

/**
 * 进程启动后，创建读线程，主线程转变为写线程
*/
int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: host <config_file_path>\n");
		return 0;
	}
	assert(Config::paser_config_file(argv[1]));
	return Shell().exec();
}