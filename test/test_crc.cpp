#include "../util.hpp"

#include <string.h>
#include <assert.h>
#include <stdio.h>

int main() {
	uint8_t buffer[1024] = "12345678129837615837647ashaywu81nasjkdhnzm a's;dkjqqiianswh90\0";
	int len = strlen((char *)buffer);
	uint16_t p = crc16_ccitt(buffer, len);
	printf("%x ", p);
	buffer[len] = (p >> 8) & 0xff;
	buffer[len + 1] = p & 0xff;
	// buffer[p % (len + 2)] = ~buffer[p % (len + 2)];
	p = crc16_ccitt(buffer, len + 2);
	printf("%x\n", p);
	assert(p == 0);
	return 0;
}