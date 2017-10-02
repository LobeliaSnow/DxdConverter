#include "FileController.hpp"
#include "DxdExporter.hpp"

//TODO : FLのSystemにあるFL::System::GetInstance()->sampleFramePerCountこれの取得方法作成
//TODO : FLにあるアニメーションテイククラスの強化

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
		if (!IsExistFile())STRICT_THROW("ファイルが開かれていない可能性があります");
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
		else STRICT_THROW("初めて見るファイルフォーマットです、この例外メッセージを制作者に連絡してください");//とかいいつつ、humanoid.fbxはこっちに来るんじゃないかなと予測
		file->Write(mesh->GetMaterialName().c_str(), 64);
		ClusterParser();
		AnimationTakeParser();
		printf("parse complete\n");
		Debug();
	}
	//ちょっと怖いのでコメントつけ倒します、後でデバッグする際勘違いが怖いので
	void Mesh::ClusterParser() {
		if (mesh->GetSkinCount() > 1)STRICT_THROW("現在、複数スキンのアニメーションには対応していません");
		FL::Skin* skin = mesh->GetSkin(0);
		//ボーン数取得
		clusterCount = skin->GetClusterCount();
		indexCount = mesh->GetIndexCount();
		//展開後の頂点の数に合わせる
		boneInfo.resize(indexCount);
		//クラスターの数に合わせる
		initPoseMatrices.resize(clusterCount);
		//アニメーション数取得 あえて下ではなくてこちらでやっている
		animationCount = skin->GetCluster(0)->GetAnimationCount();
		//この先諸データのパース ここ怪しいよ
		for (int i = 0; i < clusterCount; i++) {
			FL::Cluster* cluster = skin->GetCluster(i);
			//影響頂点数取得
			int boneIndexCount = cluster->GetIndexCount();
			for (int j = 0; j < boneIndexCount; j++) {
				BoneInfo info = {};
				//影響を受けるクラスターの影響度をセット
				info.clusterIndex = i;
				//そのクラスターが影響する重みをセット
				info.weight = cluster->GetWeight(j);
				if (info.weight<0.0f || info.weight>1.0f) {
					printf("初期せぬウェイトが取得されました\n");
					STRICT_THROW("初期せぬウェイトが取得されました");
				}
				//どの頂点が影響するか取得
				int vertexPointIndex = cluster->GetImpactIndex(j);
				//その頂点とリンクした位置へ情報をセット(後で4を超えていた場合上位4本適用)
				boneInfo[vertexPointIndex].push_back(info);
			}
			//初期姿勢行列取得
			Matrix temp = cluster->GetInitPoseMatrix();
			DirectX::XMFLOAT4X4 initPose = {};
			LoadMatrix(&initPose, temp);
			//保存用変数に初期姿勢を保存
			initPoseMatrices[i] = initPose;
		}
	}
	void Mesh::AnimationTakeParser() {
		FL::Skin* skin = mesh->GetSkin(0);
		//アニメーションがない場合、またはクラスター情報をパースしていない状態の場合何もしない
		if (animationCount <= 0)return;
		frameCount.resize(animationCount);
		animationName.resize(animationCount);
		for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
			keyFrames.resize(animationCount);
			FL::Cluster* cluster = skin->GetCluster(clusterIndex);
			DirectX::XMMATRIX initPose = {};
			//計算用の型に流す
			initPose = DirectX::XMLoadFloat4x4(&initPoseMatrices[clusterIndex]);
			//よくわからんが逆行列の算出に必要
			DirectX::XMVECTOR arg = {};
			//初期姿勢の逆行列を算出
			DirectX::XMMATRIX inverseInit = DirectX::XMMatrixInverse(&arg, initPose);
			//この使用しているFL Libraryが1秒あたり何フレームでサンプルしているか取得
			framePerCount = FL::System::GetInstance()->sampleFramePerCount;
			//複数アニメーション用にアニメーションの数だけ取得
			for (int animationIndex = 0; animationIndex < animationCount; animationIndex++) {
				keyFrames[animationIndex].resize(clusterCount);
				//アニメーションテイク取得
				FL::AnimationTake* animation = cluster->GetAnimationTake(animationIndex);
				if (animationName[animationIndex].empty())animationName[animationIndex] = animation->GetTakeName();
				//アニメーションのサンプル後のフレーム総数を取得
				//FL Libraryこの部分を関数か何か用意しておくこと
				if (frameCount[animationIndex] <= 0)frameCount[animationIndex] = animation->GetMatrices().size();
				keyFrames[animationIndex][clusterIndex].resize(frameCount[animationIndex]);
				//全フレーム分のキーを取得
				for (int keyFrameIndex = 0; keyFrameIndex < frameCount[animationIndex]; keyFrameIndex++) {
					//カレントポーズ行列を取得
					Matrix ctemp = animation->GetCurrentPoseMatrix(keyFrameIndex);
					DirectX::XMFLOAT4X4 currentPose = {};
					//行列をDirectXMathの型に入れ込む
					LoadMatrix(&currentPose, ctemp);
					DirectX::XMMATRIX current;
					//計算用の型に移し替え
					current = DirectX::XMLoadFloat4x4(&currentPose);
					//バインドポーズ行列の逆行列と、カレントポーズ行列をかけて事前計算
					//アニメーションフレームとなる
					DirectX::XMMATRIX frame = inverseInit*current;
					//保存用の型に変換、受け取りも保存用変数
					DirectX::XMStoreFloat4x4(&keyFrames[animationIndex][animationIndex][keyFrameIndex], frame);
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

		for (int i = 0; i < indexCount; i++) {
			int boneCount = boneInfo[i].size();
			fc->Print("bone info impact vertex count %d\n", boneCount);
			for (int j = 0; j < boneCount; j++) {
				fc->Print("→ cluster index %d\n", boneInfo[i][j].clusterIndex);
				fc->Print("→ weight %f\n", boneInfo[i][j].weight);
			}
		}

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
		for (int animationIndex = 0; animationIndex < animationCount; animationIndex++) {
			fc->Print("%s\n", animationName[animationIndex].c_str());
			fc->Print("frame count %d\n", frameCount[animationIndex]);
			for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
				for (int frameIndex = 0; frameIndex < frameCount[animationIndex]; frameIndex++) {
					fc->Print("key frame\n");
					for (int row = 0; row < 4; row++) {
						fc->Print("-> ");
						for (int column = 0; column < 4; column++) {
							fc->Print("%f ", keyFrames[animationIndex][clusterIndex][frameIndex].m[row][column]);
						}
						fc->Print("\n");
					}
					fc->Print("\n");
				}
			}
		}
		fc->Close();
	}
	void Mesh::ClusterExport() {
		file->Write(&clusterCount, 1);
		//一応
		file->Write(&indexCount, 1);
		for (int i = 0; i < indexCount; i++) {
			int boneInfoSize = boneInfo[i].size();
			file->Write(&boneInfoSize, 1);
			file->Write(boneInfo[i].data(), boneInfoSize);
		}
		file->Write(initPoseMatrices.data(), clusterCount);
	}
	int Mesh::GetAnimationCount() { return animationCount; }
	int Mesh::GetFrameCount(int index) { return frameCount[index]; }
	int Mesh::GetClusterCount() { return clusterCount; }
	int Mesh::GetFramePerCount() { return framePerCount; }
	std::string Mesh::GetAnimationName(int index) { return animationName[index]; }
	void Mesh::AnimationExport(std::weak_ptr<FileController> fc, int index) {
		std::shared_ptr<FileController> f = fc.lock();
		for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
			//keyFrames[アニメーションインデックス][クラスターインデックス].data()<-キーフレーム
			f->Write(keyFrames[index][clusterIndex].data(), frameCount[index]);
		}
	}

	Material::Material(FL::Material* material, std::weak_ptr<FileController> fc) :material(material), file(fc.lock()) {
	}
	Material::~Material() = default;
	bool Material::IsExistFile() { return (file.get() && file->IsOpen()); }
	void Material::Export() {
		if (!IsExistFile())STRICT_THROW("ファイルが開かれていない可能性があります");
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
		std::vector<std::unique_ptr<Mesh>> mesh;
		mesh.resize(meshCount);
		for (int i = 0; i < meshCount; i++) {
			mesh[i] = std::make_unique<Mesh>(model->GetMesh(i), fc);
			mesh[i]->MeshExport();
		}
		int materiaCount = model->GetMaterialCount();
		fc->Write(&materiaCount, 1);
		for (int i = 0; i < materiaCount; i++) {
			std::unique_ptr<Material> material;
			material = std::make_unique<Material>(model->GetMaterial(i), fc);
			material->Export();
		}
		for (int i = 0; i < meshCount; i++) {
			mesh[i]->ClusterExport();
		}
		for (int i = 0; i < mesh[0]->GetAnimationCount(); i++) {
			std::shared_ptr<FileController> fc = std::make_shared<FileController>();
			std::string fileName = mesh[0]->GetAnimationName(i) + ".anm";
			fc->Open(fileName.c_str(), FileController::OpenMode::WriteBinary);
			fc->Write(const_cast<char*>(mesh[0]->GetAnimationName(i).c_str()), 32);
			int framePerCount = mesh[0]->GetFramePerCount();
			fc->Write(&framePerCount, 1);
			int frameCount = mesh[0]->GetFrameCount(i);
			fc->Write(&frameCount, 1);
			int clusterCount = mesh[0]->GetClusterCount();
			fc->Write(&clusterCount, 1);
			for (int j = 0; j < meshCount; j++) {
				mesh[i]->AnimationExport(fc, i);
			}
			fc->Close();
		}
	}
}

