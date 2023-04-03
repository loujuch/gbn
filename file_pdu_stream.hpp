#ifndef _FILE_PDU_STREAM__
#define _FILE_PDU_STREAM__

#include "pdu.hpp"

/**
 * 打开一个文件，能以流的形式填充PDU
 * 不要求线程安全
*/
class FilePDUStream {
public:
	enum OPEN_WAY {
		READ = 0,
		WRITE
	};

	enum FILE_STREAM_STATUS {
		CLOSE = 0,
		FILE_HEAD,
		FILE_TEXT
	};
private:
	enum FILE_STREAM_STATUS status_;
	char file_name_[256];
	uint64_t file_size_;
	int file_;
	enum OPEN_WAY open_way_;
public:
	/* 创建文件帧流 */
	FilePDUStream(const char *file_name, enum OPEN_WAY way);
	/* 关闭文件帧流 */
	~FilePDUStream();

	/* 输入输出流 */
	FilePDUStream &operator<<(const PDU &pdu);
	FilePDUStream &operator>>(PDU &pdu);

	/* 是否关闭 */
	inline bool is_close() {
		return status_ == CLOSE;
	}

	/* 关闭流 */
	bool close();

	FilePDUStream(const FilePDUStream &) = delete;
	FilePDUStream &operator=(const FilePDUStream &) = delete;
};

#endif // _FILE_PDU_STREAM__