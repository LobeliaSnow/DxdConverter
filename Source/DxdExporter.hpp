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
		//�W�J��̒��_�����ɓ������l
		int indexCount;
		//�N���X�^�[�̑���
		int clusterCount;
		//���_���Ƃɉe�����󂯂�N���X�^�[�̃C���f�b�N�X����
		std::vector<std::vector<BoneInfo>> boneInfo;
		//�����p���s��ۑ��p
		std::vector<DirectX::XMFLOAT4X4> initPoseMatrices;
		//�A�j���[�V�����̐�
		int animationCount;
		//1�b������T���v�������t���[����
		int framePerCount;
		//�A�j���[�V�����̐�->�A�j���[�V������
		std::vector <std::string> animationName;
		//�A�j���[�V�������Ƃ̃t���[������
		std::vector<int> frameCount;
		//�A�j���[�V�����̐�->�t���[����->�L�[�t���[��
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