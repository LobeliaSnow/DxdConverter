#pragma once
#include <stdio.h>
#include <string>

namespace Dxd {
	class FilePathControl {
	public:
		static std::string GetExtension(const std::string& file_path);
		static std::string GetFilename(const std::string& file_path);
		static std::string GetRootDirectory(const std::string& file_path);
		static std::string GetParentDirectory(const std::string& file_path);
		static std::string GetRelativeDirectory(const std::string& file_path);
		//‘¼‚É‚à‚¢‚ë‚¢‚ë’Ç‰Á—\’è
	};
	class FileController {
	public:
		enum class OpenMode { NoOpen = -1, AppendBinaryPlus, WriteBinaryPlus, ReadBinaryPlus, AppendBinary, WriteBinary, ReadBinary, AppendPlus, WritePlus, ReadPlus, Append, Write, Read, };
		enum class SeekMode { FRONT, CURRENT, BACK };
	private:
		OpenMode mode;
		FILE* fp;
	private:
		std::string ModePurse(OpenMode m);
	public:
		FileController()noexcept;
		~FileController()noexcept;
		bool IsAppendMode()const noexcept;
		bool IsWriteMode()const noexcept;
		bool IsReadMode()const noexcept;
		bool IsBinaryMode()const noexcept;
		bool IsTextMode()const noexcept;
		bool IsOpen()const noexcept;
		OpenMode GetMode()const noexcept;
		void Open(std::string filename, OpenMode m);
		void Close()noexcept;
		template<typename T> size_t Write(T* pointer, size_t count)noexcept;
		template<typename T> size_t Read(T* pointer, size_t buffer_size, size_t count)noexcept;
		template <typename ... Args> size_t Print(const char *format, Args const & ... args)noexcept;
		template <typename ... Args> size_t Scan(const char* pointer, Args const & ... args)noexcept;
		bool Seek(int offset, SeekMode mode);
		bool Rewind()noexcept;
	};
}

#include "FileController.inl"
