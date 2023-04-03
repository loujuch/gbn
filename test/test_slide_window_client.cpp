#include "../file_pdu_stream.hpp"
#include "../channel.hpp"
#include "../slide_window.hpp"

#include <assert.h>

int main() {
	FilePDUStream file("../test_from/test.jpg", FilePDUStream::READ);
	Channel channel(inet_addr("127.0.0.1"), 8080, 10, 10);
	uint16_t seq = 0;
	SlideWindow window(channel, file, 8, 0, 50);
	window.run();
	return 0;
}