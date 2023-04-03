#include "../file_pdu_stream.hpp"
#include "../channel.hpp"

#include <assert.h>
#include <string.h>

int main() {
	FilePDUStream file("../test_to", FilePDUStream::WRITE);
	FilePDUStream test("../test_from/test.jpg", FilePDUStream::READ);
	Channel channel(INADDR_ANY, 8080, 10, 10);
	uint16_t seq = 0;
	PDU pdu;
	PDU test_pdu;
	while(!file.is_close()) {
		channel >> pdu;
		printf("recv: %u\n", pdu.seqnum());
		if(pdu.seqnum() == seq) {
			test >> test_pdu;
			if(memcmp(test_pdu.data(), pdu.data(), pdu.data_size())) {
				printf("==========================%u===========================\n", seq);
			}
			file << pdu;
			printf("recv valid: %u\n", pdu.seqnum());
			++seq;
		}
		pdu.set_ack();
		/* 返回当前需要的帧 */
		pdu.set_seqnum(seq);
		channel << pdu;
		printf("send: %u\n", pdu.seqnum());
	}
	return 0;
}