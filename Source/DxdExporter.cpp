#include "FileController.hpp"
#include "DxdExporter.hpp"

//TODO : FL��System�ɂ���FL::System::GetInstance()->sampleFramePerCount����̎擾���@�쐬
//TODO : FL�ɂ���A�j���[�V�����e�C�N�N���X�̋���

namespace Dxd {
	namespace {
		void LoadMatrix(DirectX::XMFLOAT4X4* mat0, const Matrix& mat1) {
			for (int row = 0; row < 4; row++) {
				for (int column = 0; column < 4; column++) {
					mat0->m[row][column] = mat1.mat[row][column];
				}
			}
		}
	}
	Mesh::Mesh(FL::Mesh* mesh, std::weak_ptr<FileController> fc) :mesh(mesh), file(fc.lock()) {
	}
	Mesh::~Mesh() {

	}
	bool Mesh::IsExistFile() { return (file.get() && file->IsOpen()); }
	void Mesh::MeshExport() {
		if (!IsExistFile())STRICT_THROW("�t�@�C�����J����Ă��Ȃ��\��������܂�");
		int indexCount = mesh->GetIndexCount();
		int uvCount = mesh->GetUVCount();
		file->Write(&indexCount, 1);
		std::vector<Vertex> vertices;
		if (indexCount == uvCount) {
			vertices.resize(indexCount);
			for (int i = 0; i < indexCount; i++) {
				int index = mesh->GetIndexBuffer(i);
				memcpy_s(&vertices[i].pos, sizeof(Vector4), &mesh->GetVertex(index), sizeof(Vector3));
				memcpy_s(&vertices[i].normal, sizeof(Vector4), &mesh->GetNormal(index), sizeof(Vector3));
				memcpy_s(&vertices[i].tex, sizeof(Vector2), &mesh->GetUV(i), sizeof(Vector2));
			}
			file->Write(vertices.data(), indexCount);
		}
		else STRICT_THROW("���߂Č���t�@�C���t�H�[�}�b�g�ł��A���̗�O���b�Z�[�W�𐧍�҂ɘA�����Ă�������");//�Ƃ������Ahumanoid.fbx�͂������ɗ���񂶂�Ȃ����ȂƗ\��
		file->Write(mesh->GetMaterialName().c_str(), 64);
		ClusterParser();
		AnimationTakeParser();
		printf("parse complete\n");
		Debug();
	}
	//������ƕ|���̂ŃR�����g���|���܂��A��Ńf�o�b�O����ۊ��Ⴂ���|���̂�
	void Mesh::ClusterParser() {
		if (mesh->GetSkinCount() > 1)STRICT_THROW("���݁A�����X�L���̃A�j���[�V�����ɂ͑Ή����Ă��܂���");
		FL::Skin* skin = mesh->GetSkin(0);
		//�{�[�����擾
		clusterCount = skin->GetClusterCount();
		indexCount = mesh->GetIndexCount();
		//�W�J��̒��_�̐��ɍ��킹��
		boneInfo.resize(indexCount);
		//�N���X�^�[�̐��ɍ��킹��
		initPoseMatrices.resize(clusterCount);
		//�A�j���[�V�������擾 �����ĉ��ł͂Ȃ��Ă�����ł���Ă���
		animationCount = skin->GetCluster(0)->GetAnimationCount();
		//���̐揔�f�[�^�̃p�[�X
		for (int i = 0; i < clusterCount; i++) {
			FL::Cluster* cluster = skin->GetCluster(i);
			//�e�����_���擾
			int boneIndexCount = cluster->GetIndexCount();
			for (int j = 0; j < boneIndexCount; j++) {
				BoneInfo info = {};
				//�e�����󂯂�N���X�^�[�̉e���x���Z�b�g
				info.clusterIndex = i;
				//���̃N���X�^�[���e������d�݂��Z�b�g
				info.weight = cluster->GetWeight(j);
				//�ǂ̒��_���e�����邩�擾
				int vertexPointIndex = cluster->GetImpactIndex(j);
				//���̒��_�ƃ����N�����ʒu�֏����Z�b�g(���4�𒴂��Ă����ꍇ���4�{�K�p)
				boneInfo[vertexPointIndex].push_back(info);
			}
			//�����p���s��擾
			Matrix temp = cluster->GetInitPoseMatrix();
			DirectX::XMFLOAT4X4 initPose = {};
			LoadMatrix(&initPose, temp);
			//�ۑ��p�ϐ��ɏ����p����ۑ�
			initPoseMatrices[i] = initPose;
		}
	}
	void Mesh::AnimationTakeParser() {
		FL::Skin* skin = mesh->GetSkin(0);
		//�A�j���[�V�������Ȃ��ꍇ�A�܂��̓N���X�^�[�����p�[�X���Ă��Ȃ���Ԃ̏ꍇ�������Ȃ�
		if (animationCount <= 0)return;
		frameCount.resize(animationCount);
		for (int i = 0; i < clusterCount; i++) {
			FL::Cluster* cluster = skin->GetCluster(i);
			DirectX::XMMATRIX initPose = {};
			//�v�Z�p�̌^�ɗ���
			initPose = DirectX::XMLoadFloat4x4(&initPoseMatrices[i]);
			//�悭�킩��񂪋t�s��̎Z�o�ɕK�v
			DirectX::XMVECTOR arg = {};
			//�����p���̋t�s����Z�o
			DirectX::XMMATRIX inverseInit = DirectX::XMMatrixInverse(&arg, initPose);
			//���̎g�p���Ă���FL Library��1�b�����艽�t���[���ŃT���v�����Ă��邩�擾
			framePerCount = FL::System::GetInstance()->sampleFramePerCount;
			keyFrames.resize(animationCount); animationName.resize(animationCount);
			//�����A�j���[�V�����p�ɃA�j���[�V�����̐������擾
			for (int j = 0; j < animationCount; j++) {
				//�A�j���[�V�����e�C�N�擾
				FL::AnimationTake* animation = cluster->GetAnimationTake(j);
				animationName[j] = animation->GetTakeName();
				//�A�j���[�V�����̃T���v����̃t���[���������擾
				//FL Library���̕������֐��������p�ӂ��Ă�������
				frameCount[j] = animation->GetMatrices().size();
				keyFrames[j].resize(frameCount[j]);
				//�S�t���[�����̃L�[���擾
				for (int k = 0; k < frameCount[j]; k++) {
					//�J�����g�|�[�Y�s����擾
					Matrix ctemp = animation->GetCurrentPoseMatrix(j);
					DirectX::XMFLOAT4X4 currentPose = {};
					//�s���DirectXMath�̌^�ɓ��ꍞ��
					LoadMatrix(&currentPose, ctemp);
					DirectX::XMMATRIX current;
					//�v�Z�p�̌^�Ɉڂ��ւ�
					current = DirectX::XMLoadFloat4x4(&currentPose);
					//�o�C���h�|�[�Y�s��̋t�s��ƁA�J�����g�|�[�Y�s��������Ď��O�v�Z
					//�A�j���[�V�����t���[���ƂȂ�
					DirectX::XMMATRIX frame = inverseInit*current;
					//�ۑ��p�̌^�ɕϊ��A�󂯎����ۑ��p�ϐ�
					DirectX::XMStoreFloat4x4(&keyFrames[j][k], frame);
				}
			}
		}
	}
	void Mesh::Debug() {
		std::unique_ptr<FileController> fc = std::make_unique<FileController>();
		fc->Open("debug.txt", FileController::OpenMode::Write);
		fc->Print("animation debug\n");
		fc->Print("index count %d\n", indexCount);
		fc->Print("cluster count %d\n", clusterCount);
		float weightSum = 0.0f;
		for (int i = 0; i < indexCount; i++) {
			int boneCount = boneInfo[i].size();
			fc->Print("bone info impact vertex count %d\n", boneCount);
			for (int j = 0; j < boneCount; j++) {
				fc->Print("�� cluster index %d\n", boneInfo[i][j].clusterIndex);
				fc->Print("�� weight %f\n", boneInfo[i][j].weight);
			}
		}
		fc->Print("weight sum %f\n", weightSum);

		fc->Print("init pose matirces\n");
		for (int i = 0; i < clusterCount; i++) {
			for (int row = 0; row < 4; row++) {
				fc->Print("-> ");
				for (int column = 0; column < 4; column++) {
					fc->Print("%f ", initPoseMatrices[i].m[row][column]);
				}
				fc->Print("\n");
			}
			fc->Print("\n");
		}
		fc->Print("frame per sec %d\n", framePerCount);
		fc->Print("animation count %d\n", animationCount);
		for (int i = 0; i < animationCount; i++) {
			fc->Print("%s\n", animationName[i].c_str());
			fc->Print("frame count %d\n", frameCount[i]);
			for (int j = 0; j < frameCount[i]; j++) {
				fc->Print("key frame\n");
				for (int row = 0; row < 4; row++) {
					fc->Print("-> ");
					for (int column = 0; column < 4; column++) {
						fc->Print("%f ", keyFrames[i][j].m[row][column]);
					}
					fc->Print("\n");
				}
				fc->Print("\n");
			}
		}
		fc->Close();
	}
	Material::Material(FL::Material* material, std::weak_ptr<FileController> fc) :material(material), file(fc.lock()) {
	}
	Material::~Material() = default;
	bool Material::IsExistFile() { return (file.get() && file->IsOpen()); }
	void Material::Export() {
		if (!IsExistFile())STRICT_THROW("�t�@�C�����J����Ă��Ȃ��\��������܂�");
		file->Write(material->GetName().c_str(), 64);
		file->Write(material->GetTexture(0).c_str(), 256);
		Vector3 diffuse = material->GetDiffuse();
		Vector3 ambient = material->GetAmbient();
		Vector3 specular = material->GetSpecular();
		file->Write(&diffuse, 1);
		file->Write(&ambient, 1);
		file->Write(&specular, 1);
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
			mesh->MeshExport();
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

