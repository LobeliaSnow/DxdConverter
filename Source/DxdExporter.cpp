#include "FileController.hpp"
#include "DxdExporter.hpp"

namespace Dxd {
	Mesh::Mesh(FL::Mesh* mesh, std::weak_ptr<FileController> fc) :mesh(mesh), file(fc.lock()) {
	}
	Mesh::~Mesh() {

	}
	bool Mesh::IsExistFile() { return (file.get() && file->IsOpen()); }
	void Mesh::Export() {
		if (!IsExistFile())STRICT_THROW("ファイルが開かれていない可能性があります");
		int indexCount = mesh->GetIndexCount();
		int uvCount = mesh->GetUVCount();
		file->Write(&indexCount, 1);
		std::vector<Vertex> vertices;
		if (indexCount == uvCount) {
			vertices.resize(indexCount);
			//indexとUVの数が同じ、つまり頂点バッファに展開する必要がある
			for (int i = 0; i < indexCount; i++) {
				int index = mesh->GetIndexBuffer(i);
				memcpy_s(&vertices[i].pos, sizeof(Vector4), &mesh->GetVertex(index), sizeof(Vector3));
				memcpy_s(&vertices[i].normal, sizeof(Vector4), &mesh->GetNormal(index), sizeof(Vector3));
				memcpy_s(&vertices[i].tex, sizeof(Vector2), &mesh->GetUV(index), sizeof(Vector2));
			}
			file->Write(vertices.data(), indexCount);
		}
		else STRICT_THROW("初めて見るファイルフォーマットです、この例外メッセージを制作者に連絡してください");
		file->Write(mesh->GetMaterialName().c_str(), 64);
	}
	Material::Material(FL::Material* material, std::weak_ptr<FileController> fc) :material(material), file(fc.lock()) {
	}
	Material::~Material() = default;
	bool Material::IsExistFile() { return (file.get() && file->IsOpen()); }
	void Material::Export() {
		if (!IsExistFile())STRICT_THROW("ファイルが開かれていない可能性があります");
		file->Write(material->GetName().c_str(), 64);
		file->Write(material->GetTexture(0).c_str(), 256);
	}

	DxdExporter::DxdExporter(const char* file_path) :model(std::make_unique<FL::Model>(file_path)), fc(std::make_shared<FileController>()) {
	}
	DxdExporter::~DxdExporter() = default;
	float DxdExporter::GetVersion() { return 1.00f; }
	void DxdExporter::Export(const char* file_path) {
		fc->Open(file_path, FileController::OpenMode::WriteBinary);
		int meshCount = model->GetMeshCount();
		fc->Write(&meshCount, 1);
		for (int i = 0; i < meshCount; i++) {
			std::unique_ptr<Mesh> mesh;
			mesh = std::make_unique<Mesh>(model->GetMesh(i), fc);
			mesh->Export();
		}
		int materiaCount = model->GetMaterialCount();
		fc->Write(&materiaCount, 1);
		for (int i = 0; i < materiaCount; i++) {
			std::unique_ptr<Material> material;
			material = std::make_unique<Material>(model->GetMaterial(i), fc);
			material->Export();
		}
		fc->Close();
	}
}
//test_stage_3.fbx