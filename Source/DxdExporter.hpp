#pragma once
#include "FbxLoader.h"
#include <memory>
#include <DirectXMath.h>

#pragma comment(lib,"FL Library.lib")

namespace Dxd {

	class Mesh {
	private:
		struct Vertex {
			Vector4 pos;
			Vector4 normal;
			Vector2 tex;
		};
		struct BoneInfo {
			int clusterIndex;
			float weight;
		};
	private:
		FL::Mesh* mesh;
		std::shared_ptr<FileController> file;
	private:
		//展開後の頂点総数に等しい値
		int indexCount;
		//クラスターの総数
		int clusterCount;
		//頂点ごとに影響を受けるクラスターのインデックス所持
		std::vector<std::vector<BoneInfo>> boneInfo;
		//初期姿勢行列保存用
		std::vector<DirectX::XMFLOAT4X4> initPoseMatrices;
		//アニメーションの数
		int animationCount;
		//1秒あたりサンプルされるフレーム数
		int framePerCount;
		//アニメーションの数->アニメーション名
		std::vector <std::string> animationName;
		//アニメーションごとのフレーム総数
		std::vector<int> frameCount;
		//アニメーションの数->フレーム数->キーフレーム
		std::vector<std::vector<DirectX::XMFLOAT4X4>> keyFrames;
	private:
		bool IsExistFile();
		void ClusterParser();
		void AnimationTakeParser();
		void Debug();
	public:
		Mesh(FL::Mesh* mesh, std::weak_ptr<FileController> fc);
		~Mesh();
		void MeshExport();
		void ClusterExport();
		void AnimationExport();
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