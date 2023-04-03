#include "../file_pdu_stream.hpp"
#include "../channel.hpp"

int main() {
	FilePDUStream file("../test_to", FilePDUStream::WRITE);
	Channel channel(INADDR_ANY, 8080, 10, 10);
	PDU pdu;
	while(!file.is_close()) {
		channel >> pdu;
		file << pdu;
		pdu.set_ack();
		channel << pdu;
	}
	return 0;
}