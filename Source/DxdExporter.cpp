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
		char extension[4] = "Dxd";
		fc->Write(extension, sizeof("Dxd"));
		fc->Write(&version, 1);
		int meshCount = model->GetMeshCount();
		fc->Write(&meshCount, 1);
		for (int i = 0; i < meshCount; i++) {
			FL::Mesh* mesh = model->GetMesh(i);
			int vertexCount = mesh->GetVertexCount();
			std::vector<Vector3> vertices;
			for (int j = 0; j < vertexCount; j++) {
				vertices.push_back(mesh->GetVertex(j));
			}
			int normalCount = mesh->GetNormalCount();
			std::vector<Vector3> normals;
			for (int j = 0; j < normalCount; j++) {
				normals.push_back(mesh->GetNormal(j));
			}
			int uvCount = mesh->GetUVCount();
			
		}
		fc->Close();
	}

}