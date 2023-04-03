#include "../file_pdu_stream.hpp"

int main() {
	FilePDUStream in("../test_from/test.jpg", FilePDUStream::READ);
	FilePDUStream out("../test_to/", FilePDUStream::WRITE);
	PDU pdu;
	while(!in.is_close()) {
		in >> pdu;
		out << pdu;
	}
	return 0;
}