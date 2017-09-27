#include "FileController.hpp"
#include "DxdExporter.hpp"

int main(int argc, char *argv[]) {
	try {
		char filePath[256] = {};

		if (argc <= 1) 	std::cin >> filePath;
		else strcpy_s(filePath, argv[1]);
		std::cout << filePath << std::endl;
		std::cout << "Model ->" << filePath << "のロードを開始" << std::endl;
		std::unique_ptr<Dxd::DxdExporter> exporter = std::make_unique<Dxd::DxdExporter>(filePath);
		exporter->Export("a.dxd");
		std::cout << "ロード完了" << std::endl;

		 
	}
	catch (const FL::Exception& exception) {
		exception.BoxMessage();
		return -1;
	}
	rewind(stdin);
	getchar();
	return 0;
}