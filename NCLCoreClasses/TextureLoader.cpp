/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "TextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION

#include "./stb/stb_image.h"

#include "Assets.h"

using namespace NCL;
using namespace Rendering;

std::map<std::string, TextureLoadFunction> TextureLoader::fileHandlers;

bool TextureLoader::LoadTexture(const std::string& filename, char*& outData, int& width, int &height, int &channels, int&flags) {
	if (filename.empty()) {
		std::cout << "fail1"+filename << std::endl;
		return false;
	}

	if(unsigned char* inData = LoadTexFile(filename, outData, &width, &height, &channels, &flags)) {
		outData = (char*)inData;
		return true;
	}

	std::filesystem::path path(filename);
	
	std::string extension = path.extension().string();

	bool isAbsolute = path.is_absolute();

	auto it = fileHandlers.find(extension);

	std::string realPath = isAbsolute ? filename : Assets::TEXTUREDIR + filename;

	if (it != fileHandlers.end()) {
		//There's a custom handler function for this, just use that
		return it->second(realPath, outData, width, height, channels, flags);
	}
	//By default, attempt to use stb image to get this texture
	stbi_uc *texData = stbi_load(realPath.c_str(), &width, &height, &channels, 4); //4 forces this to always be rgba!

	channels = 4; //it gets forced, we don't care about the 'real' channel size

	if (texData) {
		outData = (char*)texData;
		std::cout << "success"+filename << std::endl;
		if (filename.substr(0, 8) == "/Cubemap") {
			CreateTexFile(filename, outData, width, height, channels, flags);
		}
		return true;
	}
	std::cout << "fail2"+ filename << std::endl;
	return false;
}

void TextureLoader::RegisterTextureLoadFunction(TextureLoadFunction f, const std::string&fileExtension) {
	fileHandlers.insert(std::make_pair(fileExtension, f));
}

std::string TextureLoader::GetFileExtension(const std::string& fileExtension) {
	std::filesystem::path p = std::filesystem::path(fileExtension);

	std::filesystem::path ext = p.extension();

	return ext.string();
}

void TextureLoader::DeleteTextureData(char* data) {
	free(data);
}

unsigned int NCL::TextureLoader::StrToUInt(const char* str, int strLen) {
	unsigned int num = 0;

	for (; strLen > 0; strLen--) {
		int digit = *str - '0';
		if (digit < 0 || digit > 9) return -1;
		num = num * 10 + digit;
		str++;
	}

	return num;
}

void TextureLoader::CreateTexFile(const std::string& filename, char*& outdata, int& width, int& height, int& channels, int& flags) {
	std::string txtrFilename = filename.substr(0, filename.find('.')) + ".txtr";

	std::filesystem::path path(txtrFilename);

	bool isAbsolute = path.is_absolute();

	std::string realPath = isAbsolute ? txtrFilename : Assets::TEXTUREDIR + txtrFilename;

	std::ofstream newfile(realPath, std::ios::binary);

	newfile << width << "," << height << "," << channels << "," << flags << ",\n";

	int i = 0;

	while(outdata[i] != '\0') {
		newfile << outdata[i];
		i++;
	}

	newfile.close();
}

unsigned char* TextureLoader::LoadTexFile(const std::string& filename, char*& outData, int* width, int* height, int* channels, int* flags) {
	std::string txtrFilename = filename.substr(0, filename.find('.')) + ".txtr";

	std::filesystem::path path(txtrFilename);

	bool isAbsolute = path.is_absolute();

	std::string realPath = isAbsolute ? txtrFilename : Assets::TEXTUREDIR + txtrFilename;

	std::ifstream f(realPath, std::ios::binary);

	if(f) {
		std::string line;
		std::getline(f, line);

		int temp = 0;

		int count = 0;

		char* tempStr = new char[14];
		tempStr[13] = '\0';
		int strLen = 0;

		for(char c : line) {
			switch(c){
			case ',':
				temp = StrToUInt(tempStr, strLen);
				switch(count) {
				case 0:
					*width = temp;
					break;
				case 1:
					*height = temp;
					break;
				case 2:
					*channels = temp;
					break;
				case 3:
					*flags = temp;
					break;
				}
				count++;
				strLen = 0;
				break;
			default:
				tempStr[strLen] = c;
				strLen++;
				break;
			}
		}

		std::string tempData;

		std::getline(f, line);
		tempData += line;

		while(getline(f, line)) {
			tempData += '\n';
			tempData += line;
		}
		
		unsigned char* returnData = new unsigned char[tempData.length() + 1];

		for(int i = 0; i < tempData.length(); i++) {
			returnData[i] = static_cast<unsigned char>(tempData[i]);
		}

		returnData[tempData.length()] = '\0';

		tempStr = nullptr;
		delete[] tempStr;

		return returnData;
	}

	return nullptr;
}
