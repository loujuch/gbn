#include "receiver.hpp"

#include <memory>

#include "file_pdu_stream.hpp"
#include "log.hpp"

Receiver::Receiver(uint16_t port, uint8_t error_rate, uint8_t lost_rate):
	channel_(INADDR_ANY, port, error_rate, lost_rate) {
}

void Receiver::run() {
	auto func = [this]() {
		std::shared_ptr<FilePDUStream> file(
			new FilePDUStream(Config::file_root.c_str(), FilePDUStream::WRITE)
		);

		bool valid = false;
		uint16_t seq = 0;

		while(true) {
			PDU pdu;
			if(file->is_close()) {
				bool finish = false;
				valid = false;
				file.reset(new FilePDUStream(Config::file_root.c_str(), FilePDUStream::WRITE));
				RecvLog::hope_recv(-1);
				do {
					channel_ >> pdu;
					finish = pdu.is_ack();
					pdu.set_ack();
					pdu.set_seqnum(seq);
					channel_ << pdu;
					RecvLog::now_recv(-1);
					RecvLog::record(false);
				} while(!finish);

				RecvLog::close_log();
			}
			channel_ >> pdu;
			if(pdu.is_ack()) {
				channel_ << pdu;
			}
			if(pdu.is_init()) {
				if(!valid) {
					RecvLog::new_log();
					RecvLog::hope_recv(pdu.seqnum());
					RecvLog::now_recv(pdu.seqnum());
					RecvLog::record(false);

					seq = pdu.seqnum() + 1;
					*file << pdu;
					pdu.set_ack();
					channel_ << pdu;
					valid = true;

					RecvLog::hope_recv(pdu.seqnum() + 1);
				}
				continue;
			}
			RecvLog::now_recv(pdu.seqnum());
			RecvLog::record(false);
			if(pdu.seqnum() == seq) {
				++seq;
				RecvLog::hope_recv(seq);
				*file << pdu;
			}
			pdu.set_ack();
			pdu.set_seqnum(seq);
			channel_ << pdu;
		}
	};

	thread_ = std::thread(func);
	thread_.detach();
}