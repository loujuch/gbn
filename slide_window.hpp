#ifndef _SLIDE_WINDOW__
#define _SLIDE_WINDOW__

#include <deque>
#include <mutex>

#include "channel.hpp"
#include "file_pdu_stream.hpp"
#include "timer.hpp"

class SlideWindow {
	Channel &channel_;
	FilePDUStream &file_stream_;
	std::deque<PDU> window_;
	uint64_t window_size_;
	uint16_t seq_no_;
	uint64_t timeout_sec_;
	uint64_t timeout_nsec_;
	Timer timer_;
	std::mutex mutex_;

	/* 目标帧位置 */
	inline int64_t site(uint16_t seq) {
		return (seq + window_size_ >= seq_no_ && seq < seq_no_) ? window_size_ - (seq_no_ - seq) : -1;
	}

	/*  */
	bool add_window();
	/*  */
	void sub_window();
public:
	SlideWindow(Channel &channel, FilePDUStream &file_stream,
		uint64_t window_size, uint16_t init_seq_no, uint64_t timeout_);

	/* 运行滑动窗口 */
	void run();
};

#endif // _SLIDE_WINDOW__