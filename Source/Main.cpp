#include "FileController.hpp"
#include "DxdExporter.hpp"

int main(int argc, char *argv[]) {
	try {
		char filePath[256] = {};

		if (argc <= 1) 	std::cin >> filePath;
		else strcpy_s(filePath, argv[1]);
		FL::System::GetInstance()->Initialize();
		std::cout << "Model ->" << filePath << "�̃��[�h���J�n" << std::endl;
		std::unique_ptr<Dxd::DxdExporter> exporter = std::make_unique<Dxd::DxdExporter>(filePath);
		std::cout << "���[�h����" << std::endl;
		std::cout << "�G�N�X�|�[�g�J�n" << std::endl;
		exporter->Export("a.dxd");
		std::cout << "�G�N�X�|�[�g����" << std::endl;
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