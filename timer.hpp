#ifndef _TIMER_HPP__
#define _TIMER_HPP__

#include <deque>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>

class TimerNode {
	uint64_t timeout_sec_;
	uint64_t timeout_nsec_;
public:
	uint16_t seq_no;
	itimerspec time;

	TimerNode(uint16_t seq, uint64_t timeout_sec, uint64_t timeout_nsec);
	TimerNode(const TimerNode &timer) = default;
	TimerNode &operator=(const TimerNode &timer) = default;

	void reset();
};

/**
 * 计时器类，要求触发时间戳递增
 * 采用回调的方式运行程序，回调函数在另一线程中进行
 * 不要求线程安全
*/
class Timer {
	/* 计时器关闭标志 */
	bool is_close;
	/* 计时器描述符 */
	int timerfd_;
	/* 关闭事件描述符 */
	int timer_close_eventfd_;
	int work_close_eventfd_;
	/* 工作事件描述符 */
	int work_eventfd_;
	/* 当前正在计时事件（防止已删除计时事件在临界情况下就绪并被执行） */
	TimerNode wait_timer_;
	/* 工作线程 */
	std::thread work_thread_;
	/* 计时线程 */
	std::thread timer_thread_;
	/* 回调函数 */
	std::function<void(int)> callback_;
	/* 当前计时事件 */
	size_t now_data_;
	/* 计时事件队列 */
	std::deque<TimerNode> data_;
	/* 工作事件队列 */
	std::queue<uint16_t> work_arg_;
	/* 计时锁 */
	std::mutex mutex_;
public:
	/* 初始化计时器 */
	Timer();
	Timer(std::function<void(int)> callback);
	~Timer();

	inline void set(std::function<void(int)> callback) {
		callback_ = callback;
	}

	/* 开始计时器，包括创建两个线程 */
	void run();
	/* 关闭计时器和两个线程 */
	void close();

	/* 添加计时器事件 */
	void add(uint16_t seq_num, uint64_t timeout_sec, uint64_t timeout_nsec);
	/* 删除添加时间最早的计时器事件 */
	void del();
	/* 删除计时器事件 */
	void delto(uint16_t seq_num);

	Timer(const Timer &) = delete;
	Timer &operator=(const Timer &) = delete;
};

#endif // _TIMER_HPP__