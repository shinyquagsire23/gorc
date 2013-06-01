#pragma once

#include <boost/filesystem/path.hpp>

namespace Gorc {
namespace IO {

class ReadOnlyFile {
public:
	const boost::filesystem::path Filename;

	ReadOnlyFile(const boost::filesystem::path& filename);
	virtual ~ReadOnlyFile();

	virtual void Read(void* dest, size_t size) = 0;

	template <typename T> T Read() {
		T retval;
		Read(&retval, sizeof(T));
		return retval;
	}

	virtual void Seek(long offset) = 0;
	virtual void SetPosition(size_t offset) = 0;
	virtual size_t GetPosition() = 0;

	virtual size_t GetSize() = 0;
	virtual bool IsEndOfFile() = 0;
};

}
}