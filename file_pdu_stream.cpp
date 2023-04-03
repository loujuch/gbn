#include "file_pdu_stream.hpp"

#include "util.hpp"

#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <arpa/inet.h>

static ssize_t write_file(int fd, const uint8_t *str, uint64_t size) {
	while(size) {
		ssize_t n = write(fd, str, size);
		assert(n >= 0);
		size -= n;
		str += n;
	}
	return size;
}

static ssize_t read_file(int fd, uint8_t *str, uint64_t size) {
	while(size) {
		ssize_t n = read(fd, str, size);
		assert(n >= 0);
		size -= n;
		str += n;
	}
	return size;
}


FilePDUStream::FilePDUStream(const char *file_name, enum OPEN_WAY way):
	status_(FILE_HEAD),
	file_(-1),
	file_size_(0),
	open_way_(way) {
	assert(file_name != nullptr);

	if(open_way_ == WRITE) {
		strncpy(file_name_, file_name, sizeof(file_name_));
		size_t n = strlen(file_name_);
		if(file_name_[n - 1] != '/') {
			file_name_[n] = '/';
			file_name_[n + 1] = '\0';
		}
		return;
	}

	struct stat tmp;
	assert(stat(file_name, &tmp) == 0);
	file_size_ = tmp.st_size;

	size_t len = strlen(file_name);
	const char *p = file_name + len;
	while(*p != '/' && p != file_name) {
		--p;
	}
	p += *p == '/';
	strncpy(file_name_, p, sizeof(file_name_));

	file_ = open(file_name, O_RDONLY);
	assert(file_ >= 0);
}

FilePDUStream::~FilePDUStream() {
	if(file_ != -1) {
		::close(file_);
	}
	status_ = CLOSE;
}

FilePDUStream &FilePDUStream::operator<<(const PDU &pdu) {
	if(open_way_ != WRITE || status_ == CLOSE) {
		return *this;
	}
	uint64_t size = pdu.data_size();
	const uint8_t *p = pdu.data();
	if(status_ == FILE_HEAD) {
		memcpy(&file_size_, p, 8);
		file_size_ = ntohll(file_size_);
		p += 8;
		char buffer[512];
		snprintf(buffer, sizeof(buffer), "%s%s", file_name_, p);
		file_ = open(buffer, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		assert(file_ >= 0);
		status_ = FILE_TEXT;
	} else if(status_ == FILE_TEXT) {
		size = std::min(size, file_size_);
		ssize_t n = write_file(file_, p, size);
		assert(n == 0);
		file_size_ -= size;
		if(file_size_ == 0) {
			close();
		}
	}
	return *this;
}

FilePDUStream &FilePDUStream::operator>>(PDU &pdu) {
	if(open_way_ != READ || file_ == -1 || status_ == CLOSE) {
		return *this;
	}
	uint64_t size = pdu.data_size();
	uint8_t *p = pdu.data();
	if(status_ == FILE_HEAD) {
		uint64_t tmp = file_size_;
		tmp = htonll(tmp);
		memcpy(p, &tmp, 8);
		p += 8;
		strncpy((char *)p, file_name_, sizeof(file_name_) - 8);
		status_ = FILE_TEXT;
	} else if(status_ == FILE_TEXT) {
		size = std::min(size, file_size_);
		ssize_t n = read_file(file_, p, size);
		assert(n == 0);
		file_size_ -= size;
		if(file_size_ == 0) {
			close();
		}
	}
	return *this;
}

bool FilePDUStream::close() {
	int n = 0;
	if(file_ != -1) {
		n = ::close(file_);
	}
	if(n == 0) {
		status_ = CLOSE;
	}
	return n == 0;
}