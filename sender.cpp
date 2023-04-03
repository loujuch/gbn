#include "sender.hpp"

#include "timer.hpp"
#include "log.hpp"

Sender::Sender(in_addr_t ip, uint16_t port, const char *file_path, uint16_t seq, uint64_t timeout):
	timeout_(timeout),
	channel_(ip, port, Config::error_rate, Config::lost_rate),
	file_stream_(file_path, FilePDUStream::READ),
	seq_(seq) {
}

void Sender::run() {
	auto func = [this]() {
		PDU in, out;

		Timer timer([&](uint16_t re_seq_no) {
			SendLog::new_seq(re_seq_no);
			SendLog::record(true);
			channel_ << out;
			});

		uint64_t timeout_sec = timeout_ / 1000, timeout_nsec = (timeout_ % 1000) * 1000000;

		/* 发出初始化报文 */
		file_stream_ >> out;
		out.set_init();
		out.set_seqnum(seq_);
		channel_ << out;
		timer.add(seq_, timeout_sec, timeout_nsec);

		SendLog::new_log();
		SendLog::new_seq(seq_);
		SendLog::recv_ack(-1);
		SendLog::record(false);

		channel_ >> in;
		timer.del();

		SendLog::recv_ack(in.seqnum());


		SlideWindow slide_window(channel_, file_stream_, Config::sw_size, seq_ + 1, timeout_);
		slide_window.run();

		out.set_ack();
		channel_ << out;
		timer.add(seq_, timeout_sec, timeout_nsec);
		SendLog::new_seq(seq_);
		SendLog::record(false);
		channel_ >> in;
		timer.del();
		timer.close();

		SendLog::close_log();
	};

	thread_ = std::thread(func);
}

void Sender::wait() {
	thread_.join();
}