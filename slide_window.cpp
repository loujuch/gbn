#include "slide_window.hpp"

SlideWindow::SlideWindow(Channel &channel, FilePDUStream &file_stream,
	uint64_t window_size, uint16_t init_seq_no, uint64_t timeout):
	channel_(channel),
	file_stream_(file_stream),
	window_size_(window_size),
	seq_no_(init_seq_no) {
	timeout_sec_ = timeout / 1000;
	timeout_nsec_ = (timeout % 1000) * 1000000;

	static auto func = [&](uint16_t re_seq_no) {
		mutex_.lock();
		SendLog::new_seq(re_seq_no);
		SendLog::record(true);
		int64_t re_site = site(re_seq_no);
		if(re_site != -1) {
			channel_ << window_[re_site];
		}
		mutex_.unlock();
	};
	timer_.set(func);
}

bool SlideWindow::add_window() {
	if(file_stream_.is_close()) {
		return false;
	}

	PDU pdu;
	file_stream_ >> pdu;
	pdu.set_seqnum(seq_no_);

	SendLog::new_seq(seq_no_);
	SendLog::record(false);

	mutex_.lock();
	window_.push_back(pdu);
	channel_ << pdu;
	timer_.add(seq_no_, timeout_sec_, timeout_nsec_);
	++window_size_;
	++seq_no_;
	mutex_.unlock();
	return true;
}

void SlideWindow::sub_window() {
	mutex_.lock();
	timer_.del();
	window_.pop_front();
	--window_size_;
	mutex_.unlock();
}

void SlideWindow::run() {
	timer_.run();
	uint64_t i, j = window_size_;
	window_size_ = 0;
	for(i = 0;i < j && !file_stream_.is_close();++i) {
		add_window();
	}

	PDU pdu;
	while(window_size_) {
		channel_ >> pdu;
		uint16_t n = pdu.seqnum();
		SendLog::recv_ack(n);
		while(window_size_ > 0 && window_.front().seqnum() < n) {
			sub_window();
			add_window();
		}
	}
	timer_.close();
}