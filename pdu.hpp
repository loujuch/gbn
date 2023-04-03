#ifndef _PDU_HPP__
#define _PDU_HPP__

#include <memory>

#include "config.hpp"

/**
 * PDU结构的定义
 * 流输入PDU大小由接受帧决定，流输出帧大小由配置决定
 * PDU数据段大小需要在260-4096中
*/
class PDU {
public:
	enum Type {
		NONE = 0x0,
		ACK = 0x1,
		INIT = 0x2,
	};
private:
	/* 本PDU帧数据大小 */
	uint16_t data_size_;
	/* 本PDU帧的类型 */
	uint8_t type_;
	/* 本PDU帧的序列号 */
	uint16_t seqnum_;
	/* 本PDU帧的数据 */
	std::shared_ptr<uint8_t> data_;
	/* 本PDU帧的校验码 */
	uint16_t checksum_;
public:
	/* 创建一个PDU */
	PDU(uint16_t data_size = Config::data_size);

	PDU(const PDU &) = default;
	PDU &operator=(const PDU &) = default;

	/* 序列化PDU */
	std::shared_ptr<uint8_t> serialization() const;
	/* 反序列化PDU */
	static PDU deserailization(std::shared_ptr<uint8_t> serial);

	inline uint8_t *data() {
		return data_.get();
	}

	inline const uint8_t *data() const {
		return data_.get();
	}

	inline uint16_t head_size() const {
		return sizeof(data_size_) + sizeof(type_) + sizeof(seqnum_);
	}

	inline uint16_t extra_size() const {
		return sizeof(data_size_) + sizeof(type_) + sizeof(seqnum_) + sizeof(checksum_);
	}

	inline uint16_t data_size() const {
		return data_size_;
	}

	inline uint16_t size() const {
		return data_size() + extra_size();
	}

	inline void set_seqnum(uint16_t seqnum) {
		seqnum_ = seqnum;
	}

	inline uint16_t seqnum() const {
		return seqnum_;
	}

	inline void set_checksum(uint16_t checksum) {
		checksum_ = checksum;
	}

	inline uint16_t checksum() const {
		return checksum_;
	}

	inline bool is_ack() const {
		return (type_ & ACK) != 0;
	}

	inline void set_ack() {
		type_ |= ACK;
	}

	inline void set_unack() {
		type_ &= ~ACK;
	}

	inline bool is_init() {
		return (type_ & INIT) != 0;
	}

	inline void set_init() {
		type_ |= INIT;
	}

	inline void set_uninit() {
		type_ &= ~INIT;
	}
};


#endif // _PDU_HPP__