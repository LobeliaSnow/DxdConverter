#include "FileController.hpp"
#include "DxdExporter.hpp"

namespace Dxd {
	DxdExporter::DxdExporter(const char* file_path) :model(std::make_unique<FL::Model>(file_path)), fc(std::make_unique<FileController>()) {
	}
	DxdExporter::~DxdExporter() = default;
	float DxdExporter::GetVersion() { return 1.00f; }
	void DxdExporter::Export(const char* file_path) {
		fc->Open(file_path, FileController::OpenMode::WriteBinary);
		float version = GetVersion();
		fc->Write("Dxd", 4);
		fc->Write(&version, 1);
		fc->Close();
	}

}