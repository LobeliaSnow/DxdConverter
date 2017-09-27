#include "FileController.hpp"
#include "DxdExporter.hpp"

int main(int argc, char *argv[]) {
	try {
		char filePath[256] = {};

		if (argc <= 1) 	std::cin >> filePath;
		else strcpy_s(filePath, argv[1]);
		FL::System::GetInstance()->Initialize();
		std::cout << "Model ->" << filePath << "のロードを開始" << std::endl;
		std::unique_ptr<Dxd::DxdImporter> exporter = std::make_unique<Dxd::DxdImporter>();
		exporter->Import("a.dxd");
		std::cout << "ロード完了" << std::endl;
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