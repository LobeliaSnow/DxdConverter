#include "FileController.hpp"
#include "DxdExporter.hpp"

int main(int argc, char *argv[]) {
	try {
		char filePath[256] = {};

		if (argc <= 1) 	std::cin >> filePath;
		else strcpy_s(filePath, argv[1]);
		FL::System::GetInstance()->Initialize();
		std::cout << "Model ->" << filePath << "のロードを開始" << std::endl;
		std::unique_ptr<Dxd::DxdExporter> exporter = std::make_unique<Dxd::DxdExporter>(filePath);
		std::cout << "ロード完了" << std::endl;
		std::cout << "エクスポート開始" << std::endl;
		std::string outputPath = Dxd::FilePathControl::GetFilename(filePath);
		std::string extension = Dxd::FilePathControl::GetExtension(filePath);
		{
			std::string serach = "fbx";
			std::string dxd = "dxd";
			std::string temp = outputPath;
			transform(temp.begin(), temp.end(), temp.begin(), tolower);
			std::string::size_type pos(temp.find(serach));
			outputPath.replace(pos, serach.length(), dxd);
		}
		//pos = outputPath.find(serach, pos + dxd.length());
		exporter->Export(outputPath.c_str());
		std::cout << "エクスポート完了" << std::endl;
		exporter.reset();
		FL::System::GetInstance()->Finalize();
	}
	catch (const FL::Exception& exception) {
		exception.BoxMessage();
		return -1;
	}
	rewind(stdin);
	getchar();
	return 0;
}