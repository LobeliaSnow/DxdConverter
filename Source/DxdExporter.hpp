#pragma once
#include "FbxLoader.h"
#include <memory>

#pragma comment(lib,"FL Library.lib")

namespace Dxd {
	class Mesh {
	private:
		struct Vertex {
			Vector4 pos;
			Vector4 normal;
			Vector2 tex;
		};
	private:
		FL::Mesh* mesh;
		std::shared_ptr<FileController> file;
	private:
		bool IsExistFile();
	public:
		Mesh(FL::Mesh* mesh, std::weak_ptr<FileController> fc);
		~Mesh();
		void Export();
	};
	class Material {
	private:
		FL::Material* material;
		std::shared_ptr<FileController> file;
	private:
		bool IsExistFile();
	public:
		Material(FL::Material* material, std::weak_ptr<FileController> fc);
		~Material();
		void Export();
	};

	class DxdExporter {
	private:
		std::unique_ptr<FL::Model> model;
		std::shared_ptr<FileController> fc;
	private:
		float GetVersion();
	public:
		DxdExporter(const char* file_path);
		~DxdExporter();
		void Export(const char* file_path);
	};
}