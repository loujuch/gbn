#include "../file_pdu_stream.hpp"
#include "../channel.hpp"

#include <assert.h>

int main() {
	FilePDUStream file("../test_to", FilePDUStream::WRITE);
	Channel channel(INADDR_ANY, 8080, 20, 5);
	uint16_t seq = 0;
	PDU pdu;
	while(!file.is_close()) {
		channel >> pdu;
		if(pdu.seqnum() == seq) {
			file << pdu;
			++seq;
		}
		pdu.set_ack();
		channel << pdu;
	}
	return 0;
}