#include "pdu.hpp"

#include <arpa/inet.h>
#include <string.h>

PDU::PDU(uint16_t data_size):
	data_size_(data_size),
	type_(NONE),
	seqnum_(0),
	data_(nullptr),
	checksum_(0) {
	if(data_size) {
		data_.reset(new uint8_t[data_size]);
	}
}

std::shared_ptr<uint8_t> PDU::serialization() const {
	uint8_t *buffer = new uint8_t[size()];
	uint8_t *p = buffer;
	uint16_t tmp16;
	tmp16 = htons(data_size_);
	memcpy(p, &tmp16, 2);
	p += 2;
	*p++ = type_;
	tmp16 = htons(seqnum_);
	memcpy(p, &tmp16, 2);
	p += 2;
	if(!(type_ & ACK)) {
		memcpy(p, data_.get(), data_size_);
	}
	p += data_size_;
	tmp16 = htons(checksum_);
	memcpy(p, &tmp16, 2);
	std::shared_ptr<uint8_t>ptr(buffer);
	return ptr;
}

PDU PDU::deserailization(std::shared_ptr<uint8_t> serial) {
	uint8_t *p = serial.get();
	uint16_t tmp16;
	memcpy(&tmp16, p, 2);
	p += 2;
	tmp16 = ntohs(tmp16);
	PDU pdu(tmp16);
	pdu.type_ = *p++;
	memcpy(&tmp16, p, 2);
	p += 2;
	tmp16 = ntohs(tmp16);
	pdu.seqnum_ = tmp16;
	if(!(pdu.type_ & ACK)) {
		memcpy(pdu.data_.get(), p, pdu.data_size_);
	}
	p += pdu.data_size_;

	tmp16 = p[0];
	tmp16 <<= 8;
	tmp16 |= p[1];

	pdu.checksum_ = tmp16;
	return pdu;
}