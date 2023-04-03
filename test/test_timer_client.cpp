#include "../file_pdu_stream.hpp"
#include "../channel.hpp"
#include "../timer.hpp"

#include <assert.h>

int main() {
	FilePDUStream file("../test_from/test.jpg", FilePDUStream::READ);
	Channel channel(inet_addr("127.0.0.1"), 8080, 20, 5);
	uint16_t seq = 0;
	PDU pdu;
	auto func = [&](uint16_t seq_no) {
		printf("timeout: %u\n", seq_no);
		assert(seq_no == pdu.seqnum());
		channel << pdu;
	};
	Timer timer(func);
	timer.run();
	while(!file.is_close()) {
		file >> pdu;
		pdu.set_seqnum(seq);
		channel << pdu;
		timer.add(seq, 0, 10000000);
		do {
			channel >> pdu;
		} while(pdu.seqnum() != seq);
		assert(pdu.is_ack());
		pdu.set_unack();
		timer.del(seq);
		++seq;
	}
	timer.close();
	return 0;
}