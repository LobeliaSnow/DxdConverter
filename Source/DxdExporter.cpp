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
		//この先諸データのパース
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
		for (int i = 0; i < clusterCount; i++) {
			FL::Cluster* cluster = skin->GetCluster(i);
			DirectX::XMMATRIX initPose = {};
			//計算用の型に流す
			initPose = DirectX::XMLoadFloat4x4(&initPoseMatrices[i]);
			//よくわからんが逆行列の算出に必要
			DirectX::XMVECTOR arg = {};
			//初期姿勢の逆行列を算出
			DirectX::XMMATRIX inverseInit = DirectX::XMMatrixInverse(&arg, initPose);
			//この使用しているFL Libraryが1秒あたり何フレームでサンプルしているか取得
			framePerCount = FL::System::GetInstance()->sampleFramePerCount;
			keyFrames.resize(animationCount); animationName.resize(animationCount);
			//複数アニメーション用にアニメーションの数だけ取得
			for (int j = 0; j < animationCount; j++) {
				//アニメーションテイク取得
				FL::AnimationTake* animation = cluster->GetAnimationTake(j);
				animationName[j] = animation->GetTakeName();
				//アニメーションのサンプル後のフレーム総数を取得
				//FL Libraryこの部分を関数か何か用意しておくこと
				frameCount[j] = animation->GetMatrices().size();
				keyFrames[j].resize(frameCount[j]);
				//全フレーム分のキーを取得
				for (int k = 0; k < frameCount[j]; k++) {
					//カレントポーズ行列を取得
					Matrix ctemp = animation->GetCurrentPoseMatrix(j);
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
				fc->Print("→ cluster index %d\n", boneInfo[i][j].clusterIndex);
				fc->Print("→ weight %f\n", boneInfo[i][j].weight);
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

