#include "timer.hpp"

#include <stdint.h>
#include <time.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <assert.h>

#define S_TO_NS 1000000000

TimerNode::TimerNode(uint16_t seq, uint64_t timeout_sec, uint64_t timeout_nsec):
	timeout_sec_(timeout_sec),
	timeout_nsec_(timeout_nsec),
	seq_no(seq) {
	reset();
}

void TimerNode::reset() {
	clock_gettime(CLOCK_MONOTONIC, &(time.it_value));
	time.it_value.tv_nsec += timeout_nsec_;
	time.it_value.tv_sec += timeout_sec_ + time.it_value.tv_nsec / S_TO_NS;
	time.it_value.tv_nsec %= S_TO_NS;

	time.it_interval.tv_sec = timeout_sec_;
	time.it_interval.tv_nsec = timeout_nsec_;
}

Timer::Timer():
	is_close(true),
	timerfd_(timerfd_create(CLOCK_MONOTONIC, 0)),
	timer_close_eventfd_(eventfd(0, 0)),
	work_close_eventfd_(eventfd(0, 0)),
	work_eventfd_(eventfd(0, 0)),
	now_data_(0),
	wait_timer_(0, 0, 0) {
	assert(timer_close_eventfd_ >= 0);
	assert(work_close_eventfd_ >= 0);
	assert(work_eventfd_ >= 0);
	assert(timerfd_ >= 0);
}

Timer::Timer(std::function<void(int)> callback):
	is_close(true),
	callback_(callback),
	timerfd_(timerfd_create(CLOCK_MONOTONIC, 0)),
	timer_close_eventfd_(eventfd(0, 0)),
	work_close_eventfd_(eventfd(0, 0)),
	work_eventfd_(eventfd(0, 0)),
	now_data_(0),
	wait_timer_(0, 0, 0) {
	assert(timer_close_eventfd_ >= 0);
	assert(work_close_eventfd_ >= 0);
	assert(work_eventfd_ >= 0);
	assert(timerfd_ >= 0);
}

Timer::~Timer() {
	if(!is_close) {
		is_close = true;
		eventfd_write(work_close_eventfd_, 1);
		work_thread_.join();
		eventfd_write(timer_close_eventfd_, 1);
		timer_thread_.join();
		::close(timerfd_);
		::close(work_close_eventfd_);
		::close(timer_close_eventfd_);
		::close(work_eventfd_);
	}
}

void Timer::close() {
	if(is_close) {
		return;
	}
	is_close = true;
	eventfd_write(work_close_eventfd_, 1);
	work_thread_.join();
	eventfd_write(timer_close_eventfd_, 1);
	timer_thread_.join();
	::close(timerfd_);
	::close(work_close_eventfd_);
	::close(timer_close_eventfd_);
	::close(work_eventfd_);
}

void Timer::run() {
	auto worker = [this]() {
		int epoll_fd = epoll_create1(0);
		epoll_event event[4];

		event[0].data.fd = work_close_eventfd_;
		event[0].events = EPOLLIN;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, work_close_eventfd_, &event[0]);

		event[0].data.fd = work_eventfd_;
		event[0].events = EPOLLIN;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, work_eventfd_, &event[0]);

		uint16_t seq;
		int n;
		uint64_t num;
		while(!is_close) {
			n = epoll_wait(epoll_fd, event, 4, -1);
			for(int i = 0;i < n;++i) {
				if(event->data.fd == work_close_eventfd_) {
					break;
				} else {
					eventfd_read(work_eventfd_, &num);
					while(num--) {
						seq = work_arg_.front();
						work_arg_.pop();
						uint64_t i;
						callback_(seq);
					}
				}
			}
		}

		::close(epoll_fd);
	};

	auto timer = [this]() {
		int epoll_fd = epoll_create1(0);
		epoll_event event[4];

		event[0].data.fd = timer_close_eventfd_;
		event[0].events = EPOLLIN;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_close_eventfd_, &event[0]);

		event[0].data.fd = timerfd_;
		event[0].events = EPOLLIN;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timerfd_, &event[0]);

		int n = 0;
		uint64_t tmp;

		while(!is_close) {
			n = epoll_wait(epoll_fd, event, 4, -1);
			for(int i = 0;i < n;++i) {
				if(event->data.fd == timer_close_eventfd_) {
					break;
				} else {
					read(timerfd_, &tmp, 8);
					mutex_.lock();
					// printf("timeout data_ size: %ld\n", data_.size());
					if(wait_timer_.seq_no == data_[now_data_].seq_no) {
						data_[now_data_].reset();
						now_data_ = now_data_ + 1 >= data_.size() ? 0 : now_data_ + 1;
						work_arg_.push(wait_timer_.seq_no);
						wait_timer_ = data_[now_data_];
						assert(wait_timer_.time.it_value.tv_sec != 0 ||
							wait_timer_.time.it_value.tv_nsec != 0 ||
							wait_timer_.time.it_interval.tv_sec != 0 ||
							wait_timer_.time.it_interval.tv_nsec != 0);
						timerfd_settime(timerfd_, 1, &(wait_timer_.time), nullptr);
						eventfd_write(work_eventfd_, 1);
					}
					mutex_.unlock();
				}
			}
		}

		::close(epoll_fd);
	};

	is_close = false;
	timer_thread_ = std::thread(timer);
	work_thread_ = std::thread(worker);
}

void Timer::add(uint16_t seq_num, uint64_t timeout_sec, uint64_t timeout_nsec) {
	if(is_close) {
		return;
	}
	mutex_.lock();
	data_.emplace_back(seq_num, timeout_sec, timeout_nsec);
	/* 出现计时事件，开启计时器 */
	if(data_.size() == 1) {
		now_data_ = 0;
		wait_timer_ = data_.front();
		assert(wait_timer_.time.it_value.tv_sec != 0 ||
			wait_timer_.time.it_value.tv_nsec != 0 ||
			wait_timer_.time.it_interval.tv_sec != 0 ||
			wait_timer_.time.it_interval.tv_nsec != 0);
		timerfd_settime(timerfd_, 1, &(wait_timer_.time), nullptr);
	}
	timespec tmp;
	clock_gettime(CLOCK_MONOTONIC, &tmp);
	mutex_.unlock();
}

void Timer::del() {
	if(is_close) {
		return;
	}
	mutex_.lock();
	data_.pop_front();
	if(data_.empty()) {
		wait_timer_.time.it_interval.tv_nsec = 0;
		wait_timer_.time.it_interval.tv_sec = 0;
		wait_timer_.time.it_value.tv_nsec = 0;
		wait_timer_.time.it_value.tv_sec = 0;
		timerfd_settime(timerfd_, 1, &(wait_timer_.time), nullptr);
	} else {
		if(now_data_ == 0) {
			wait_timer_ = data_[now_data_];
			assert(wait_timer_.time.it_value.tv_sec != 0 ||
				wait_timer_.time.it_value.tv_nsec != 0 ||
				wait_timer_.time.it_interval.tv_sec != 0 ||
				wait_timer_.time.it_interval.tv_nsec != 0);
			timerfd_settime(timerfd_, 1, &(wait_timer_.time), nullptr);
		} else {
			--now_data_;
		}
	}
	mutex_.unlock();
}

void Timer::delto(uint16_t seq_num) {
	if(is_close) {
		return;
	}
	uint64_t site = 0, pop_num;
	/* ACK乱序到达，要求ACK序列号正确 */
	for(site = 0;site < data_.size();++site) {
		if(seq_num == data_[site].seq_no) {
			break;
		}
	}
	++site;
	pop_num = site;
	while(pop_num--) {
		data_.pop_front();
	}
	if(data_.empty()) {
		wait_timer_.time.it_interval.tv_nsec = 0;
		wait_timer_.time.it_interval.tv_sec = 0;
		wait_timer_.time.it_value.tv_nsec = 0;
		wait_timer_.time.it_value.tv_sec = 0;
		timerfd_settime(timerfd_, 1, &(wait_timer_.time), nullptr);
	} else if(site < now_data_) {
		now_data_ = 0;
		wait_timer_ = data_[now_data_];
		assert(wait_timer_.time.it_value.tv_sec != 0 ||
			wait_timer_.time.it_value.tv_nsec != 0 ||
			wait_timer_.time.it_interval.tv_sec != 0 ||
			wait_timer_.time.it_interval.tv_nsec != 0);
		timerfd_settime(timerfd_, 1, &(wait_timer_.time), nullptr);
	} else {
		now_data_ -= site;
	}
	mutex_.unlock();
}
