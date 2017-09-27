#include "FbxLoader.h"
#pragma comment(lib,"FL Library.lib")
#include <memory>

int main(int argc, char *argv[]) {
	char filePath[256] = {};
	if (argc <= 1) 	std::cin >> filePath;
	else strcpy_s(filePath, argv[1]);
	std::cout << filePath << std::endl;
	std::unique_ptr<FL::Model> model=std::make_unique<FL::Model>(filePath);
	rewind(stdin);
	getchar();
	return 0;
}