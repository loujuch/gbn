#include "channel.hpp"

#include "util.hpp"
#include "log.hpp"

#include <sys/socket.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

Channel::Channel(in_addr_t ip, uint16_t port, uint8_t error_rate, uint8_t lose_rate):
	error_rate_(error_rate),
	lose_rate_(lose_rate),
	udp_socket_(-1),
	addr_len_(sizeof(udp_addr_)) {
	udp_socket_ = socket(PF_INET, SOCK_DGRAM, 0);
	assert(udp_socket_ >= 0);

	memset(&udp_addr_, 0, addr_len_);
	udp_addr_.sin_family = AF_INET;
	udp_addr_.sin_port = htons(port);
	udp_addr_.sin_addr.s_addr = ip;

	if(ip == INADDR_ANY) {
		assert(bind(udp_socket_, (sockaddr *)&udp_addr_, addr_len_) >= 0);
	}

	srand(time(nullptr));
}

Channel::~Channel() {
	assert(udp_socket_ != -1);
	::close(udp_socket_);
}

Channel &Channel::operator<<(const PDU &pdu) {
	assert(udp_socket_ >= 0);
	std::shared_ptr<uint8_t> ptr = pdu.serialization();

	uint16_t check = crc16_ccitt(ptr.get(), pdu.size() - 2);
	ptr.get()[pdu.size() - 2] = (check >> 8) & 0xff;
	ptr.get()[pdu.size() - 1] = check & 0xff;

	if(rand() % 100 < error_rate_) {
		ptr.get()[check % pdu.size()] = ~ptr.get()[check % pdu.size()];
	}

	ssize_t n = sendto(udp_socket_, ptr.get(), pdu.size(), 0, (sockaddr *)&udp_addr_, addr_len_);
	assert(n == pdu.size());
	return *this;
}

/* 在接收端模拟错误与丢包 */
Channel &Channel::operator>>(PDU &pdu) {
	assert(udp_socket_ >= 0);
	uint8_t *buffer = new uint8_t[4096 + pdu.extra_size()];
	ssize_t n;
	uint16_t m;
	do {
		do {
			n = recvfrom(udp_socket_, buffer, 4096 + pdu.extra_size(), 0, (sockaddr *)&udp_addr_, &addr_len_);
			assert(n >= 0);
			m = rand() % 100;
		} while(m < lose_rate_);
		m = crc16_ccitt(buffer, n);
		if(m != 0) {
			uint16_t tmp16;
			memcpy(&tmp16, buffer + 3, 2);
			tmp16 = ntohs(tmp16);
			RecvLog::now_recv(tmp16);
			RecvLog::record(true);
		}
	} while(m != 0);

	std::shared_ptr<uint8_t>ptr(buffer);
	pdu = PDU::deserailization(ptr);
	assert(n == pdu.size());
	return *this;
}