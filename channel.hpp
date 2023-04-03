#ifndef _CHANNEL_HPP__
#define _CHANNEL_HPP__

#include "pdu.hpp"

#include <stdint.h>
#include <arpa/inet.h>

/**
 * 使用UDP协议模拟有丢包和比特错误的全双工信道
 * 系统调用是原子操作，因此线程安全
*/
class Channel {
	/* 错误率 */
	uint8_t error_rate_;
	/* 丢失率 */
	uint8_t lose_rate_;
	/* UDP socket */
	int udp_socket_;
	/* 要发送的或最近接收到信息的目标信息，回信用 */
	sockaddr_in udp_addr_;
	/* udp_addr_长度 */
	socklen_t addr_len_;
public:
	/* 创建信道，若ip为INADDR_ANY，则为接受通道，需要绑定对应端口 */
	Channel(in_addr_t ip,
		uint16_t port,// = Config::udp_port,
		uint8_t error_rate,// = Config::error_rate,
		uint8_t lose_rate);// = Config::lost_rate);
	/* 关闭信道 */
	~Channel();

	/* 输出流 */
	Channel &operator<<(const PDU &pdu);
	/* 出入流 */
	Channel &operator>>(PDU &pdu);

	Channel() = delete;
	Channel(const Channel &) = delete;
	Channel &operator=(const Channel &) = delete;
};

#endif // _CHANNEL_HPP__