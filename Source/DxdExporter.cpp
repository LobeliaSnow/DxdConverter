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
			fc->Write(&vertexCount, 1);
			fc->Write(vertices.data(), vertexCount);

			int normalCount = mesh->GetNormalCount();
			std::vector<Vector3> normals;
			for (int j = 0; j < normalCount; j++) {
				normals.push_back(mesh->GetNormal(j));
			}
			fc->Write(&normalCount, 1);
			fc->Write(normals.data(), vertexCount);

			int uvCount = mesh->GetUVCount();
			std::vector<Vector2> uvs;
			for (int j = 0; j < uvCount; j++) {
				uvs.push_back(mesh->GetUV(j));
			}
			fc->Write(&uvCount, 1);
			fc->Write(uvs.data(), vertexCount);

			int indexCount = mesh->GetIndexCount();
			std::vector<int> indices;
			for (int j = 0; j < indexCount; j++) {
				indices.push_back(mesh->GetIndexBuffer(j));
			}
			fc->Write(&indexCount, 1);
			fc->Write(indices.data(), vertexCount);

			std::string materialName = mesh->GetMaterialName();
			fc->Write(materialName.c_str(), 256);
		}
		int materialCount = model->GetMaterialCount();
		fc->Write(&materialCount, 1);
		for (int i = 0; i < materialCount; i++) {
			FL::Material* material = model->GetMaterial(i);
			std::string materialName = material->GetName();
			fc->Write(materialName.c_str(), 256);

			std::string textureName = material->GetTexture(0);
			fc->Write(textureName.c_str(), 256);
		}
		fc->Close();
	}
	DxdImporter::DxdImporter() :fc(std::make_unique<FileController>()) {}
	DxdImporter::~DxdImporter() = default;
	void DxdImporter::Import(const char* file_path) {
		fc->Open(file_path, FileController::OpenMode::ReadBinary);
		float version = GetVersion();
		char extension[4] = "Dxd";
		fc->Read(extension, sizeof("Dxd"), 1);
		fc->Read(&version, sizeof(version), 1);
		int meshCount = -1;
		fc->Read(&meshCount, sizeof(meshCount), 1);
		for (int i = 0; i < meshCount; i++) {
			int vertexCount = -1;
			fc->Read(&vertexCount, sizeof(vertexCount), 1);
			std::vector<Vector3> vertices;
			vertices.resize(vertexCount);
			fc->Read(vertices.data(), sizeof(Vector3)*vertexCount, 1);

			int normalCount = -1;
			fc->Read(&normalCount, sizeof(normalCount), 1);
			std::vector<Vector3> normals;
			normals.resize(normalCount);
			fc->Read(normals.data(), sizeof(Vector3)*normalCount, 1);

			int uvCount = -1;
			fc->Read(&uvCount, sizeof(uvCount), 1);
			std::vector<Vector2> uvs;
			uvs.resize(uvCount);
			fc->Read(uvs.data(), sizeof(Vector2)*uvCount, 1);

			int indexCount = -1;
			fc->Read(&indexCount, sizeof(indexCount), 1);
			std::vector<int> indices;
			indices.resize(indexCount);
			fc->Read(indices.data(), sizeof(int)*indexCount, 1);

			char materialName[256] = {};
			fc->Read(materialName, 256, 1);
		}
		int materialCount = -1;
		fc->Read(&materialCount, sizeof(materialCount), 1);
		for (int i = 0; i < materialCount; i++) {
			char materialName[256] = {};
			fc->Read(materialName, 256, 1);

			char textureName[256] = {};
			fc->Read(textureName, 256, 1);
		}
		fc->Close();

	}

}