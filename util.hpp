#ifndef _UTIL_HPP__
#define _UTIL_HPP__

#include <stdint.h>

/* 64字节字节序变化 */
uint64_t htonll(uint64_t value);
uint64_t ntohll(uint64_t value);

/* 使用CRC16-CCITT标准进行校验，生成多项式为：x^16+x^12+x^5+1（1021）*/
uint16_t crc16_ccitt(const uint8_t *data, uint16_t size);

/* 判断当前窗口大小下是否为有效序列号 */
bool seq_vaild(uint16_t seq, uint64_t window_size, uint16_t test);

#endif // _UTIL_HPP__