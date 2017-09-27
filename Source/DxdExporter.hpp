#pragma once
#include "FbxLoader.h"
#include <memory>

#pragma comment(lib,"FL Library.lib")

namespace Dxd {
	class DxdExporter {
	private:
		std::unique_ptr<FL::Model> model;
		std::unique_ptr<FileController> fc;
	private:
		float GetVersion();
	public:
		DxdExporter(const char* file_path);
		~DxdExporter();
		void Export(const char* file_path);
	};
	class DxdImporter {
	private:
		std::unique_ptr<FileController> fc;
	public:
		DxdImporter();
		~DxdImporter();
		void Import(const char* file_path);
	};
}