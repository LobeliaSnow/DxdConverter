#include <intsafe.h>
#include <filesystem>

namespace Dxd {
	__forceinline std::string FilePathControl::GetExtension(const std::string& file_path) {
		std::tr2::sys::path path(file_path);
		if (!path.has_extension())return "";
		return path.extension().string();
	}
	__forceinline std::string FilePathControl::GetFilename(const std::string& file_path) {
		std::tr2::sys::path path(file_path);
		if (!path.has_filename())return "";
		return path.filename().string();
	}
	__forceinline std::string FilePathControl::GetRootDirectory(const std::string& file_path) {
		std::tr2::sys::path path(file_path);
		if (!path.has_root_directory())return "";
		return path.root_directory().string();
	}
	__forceinline std::string FilePathControl::GetParentDirectory(const std::string& file_path) {
		std::tr2::sys::path path(file_path);
		if (!path.has_parent_path())return "";
		return path.parent_path().string();
	}
	__forceinline std::string FilePathControl::GetRelativeDirectory(const std::string& file_path) {
		std::tr2::sys::path path(file_path);
		if (!path.has_relative_path())return "";
		return path.relative_path().string();
	}


	__forceinline std::string FileController::ModePurse(OpenMode m) {
		switch (m) {
		default:
		case OpenMode::AppendBinaryPlus:	return "ab+";
		case OpenMode::WriteBinaryPlus:		return "wb+";
		case OpenMode::ReadBinaryPlus:		return "rb+";
		case OpenMode::AppendBinary:		return "ab";
		case OpenMode::WriteBinary:			return "wb";
		case OpenMode::ReadBinary:			return "rb";
		case OpenMode::AppendPlus:			return "a+";
		case OpenMode::WritePlus:				return "w+";
		case OpenMode::ReadPlus:				return "r+";
		case OpenMode::Append:					return "a";
		case OpenMode::Write:					return "w";
		case OpenMode::Read:						return "r";
		}
	}
	__forceinline FileController::FileController()noexcept : fp(nullptr) {}
	__forceinline FileController::~FileController()noexcept = default;
	__forceinline bool FileController::IsAppendMode()const noexcept { return (mode == OpenMode::AppendBinaryPlus || mode == OpenMode::AppendBinary || mode == OpenMode::AppendPlus || mode == OpenMode::Append); }
	__forceinline bool FileController::IsWriteMode()const noexcept { return (mode == OpenMode::WriteBinaryPlus || mode == OpenMode::WriteBinary || mode == OpenMode::WritePlus || mode == OpenMode::Write); }
	__forceinline bool FileController::IsReadMode()const noexcept { return (mode == OpenMode::ReadBinaryPlus || mode == OpenMode::ReadBinary || mode == OpenMode::ReadPlus || mode == OpenMode::Read); }
	__forceinline bool FileController::IsBinaryMode()const noexcept { return (mode == OpenMode::ReadBinaryPlus || mode == OpenMode::ReadBinary || mode == OpenMode::WriteBinaryPlus || mode == OpenMode::WriteBinary || mode == OpenMode::ReadBinaryPlus || mode == OpenMode::ReadBinary); }
	__forceinline bool FileController::IsTextMode()const noexcept { return (mode == OpenMode::ReadPlus || mode == OpenMode::Read || mode == OpenMode::WritePlus || mode == OpenMode::Write || mode == OpenMode::ReadPlus || mode == OpenMode::Read); }
	__forceinline bool FileController::IsOpen()const noexcept { return (fp != nullptr); }
	__forceinline FileController::OpenMode FileController::GetMode()const noexcept { return mode; }
	__forceinline void FileController::Open(std::string filename, OpenMode m) {
		if (IsOpen())throw;
		mode = m;
		fopen_s(&fp, filename.c_str(), ModePurse(m).c_str());
		if (!fp)throw;
	}
	__forceinline void FileController::Close()noexcept {
		if (IsOpen()) {
			fclose(fp);
			fp = nullptr;
			mode = OpenMode::NoOpen;
		}
	}
	template<typename T>  size_t FileController::Write(T* pointer, size_t count) noexcept {
		if (!IsOpen() || !(IsWriteMode() || IsAppendMode()))return SIZE_T_MAX;
		return fwrite(pointer, sizeof T, count, fp);
	}
	template<typename T> __forceinline size_t FileController::Read(T* pointer, size_t buffer_size, size_t count) noexcept {
		if (!IsOpen() || !IsReadMode())return SIZE_T_MAX;
		return fread_s(pointer, buffer_size, buffer_size, count, fp);
	}
	template <typename ... Args> __forceinline size_t FileController::Print(const char *format, Args const & ... args) noexcept {
		if (!IsOpen() || !(IsWriteMode() || IsAppendMode()))return SIZE_T_MAX;
		return fprintf_s(fp, format, args...);
	}
	template <typename ... Args> __forceinline size_t FileController::Scan(const char* pointer, Args const & ... args) noexcept {
		if (!IsOpen() || !IsReadMode())return SIZE_T_MAX;
		return fscanf_s(fp, format, args...);
	}
	__forceinline bool FileController::Seek(int offset, SeekMode mode) {
		if (!IsOpen())return false;
		int origin = -1;
		switch (mode) {
		case SeekMode::FRONT:		origin = SEEK_SET;	break;
		case SeekMode::CURRENT:	origin = SEEK_CUR;	break;
		case SeekMode::BACK:			origin = SEEK_END;	break;
		default:	throw;
		}
		return (fseek(fp, offset, origin) == 0);
	}
	__forceinline bool FileController::Rewind()noexcept {
		if (!IsOpen())return false;
		rewind(fp);
		return true;
	}
}