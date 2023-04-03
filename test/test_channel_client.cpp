#include "../file_pdu_stream.hpp"
#include "../channel.hpp"

#include <assert.h>

int main() {
	FilePDUStream file("../test_from/test.jpg", FilePDUStream::READ);
	Channel channel(inet_addr("127.0.0.1"), 8080, 10, 10);
	while(!file.is_close()) {
		PDU pdu;
		file >> pdu;
		channel << pdu;
		channel >> pdu;
		assert(pdu.is_ack());
	}
	return 0;
}