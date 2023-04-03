#include "util.hpp"

#include <arpa/inet.h>

uint64_t htonll(uint64_t value) {
	uint32_t oh = htonl((value >> 32) & 0xffffffff), ol = htonl(value & 0xffffffff);
	uint64_t out = oh;
	out <<= 32;
	out |= ol;
	return out;
}

uint64_t ntohll(uint64_t value) {
	uint32_t oh = ntohl((value >> 32) & 0xffffffff), ol = ntohl(value & 0xffffffff);
	uint64_t out = oh;
	out <<= 32;
	out |= ol;
	return out;
}

uint16_t crc16_ccitt(const uint8_t *data, uint16_t size) {
	static const uint16_t crc_table[16] = {
		0x0000, 0x1021, 0x2042, 0x3063,
		0x4084, 0x50a5, 0x60c6, 0x70e7,
		0x8108, 0x9129, 0xa14a, 0xb16b,
		0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
	};

	uint16_t crc16 = 0x0000;
	uint16_t crc_h4, crc_l12;

	while(size--) {
		crc_h4 = (crc16 >> 12);
		crc_l12 = (crc16 << 4);
		crc16 = crc_l12 ^ crc_table[crc_h4 ^ (*data >> 4)];
		crc_h4 = (crc16 >> 12);
		crc_l12 = (crc16 << 4);
		crc16 = crc_l12 ^ crc_table[crc_h4 ^ (*data & 0x0f)];
		++data;
	}

	return crc16;
}

bool seq_vaild(uint16_t seq, uint64_t window_size, uint16_t test) {
	return test >= seq && test != -1;
}