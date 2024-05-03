/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "TextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "./stb/stb_image.h"
#include "./stb/stb_image_resize.h"
#include "./stb/stb_image_write.h"

#include "Assets.h"

using namespace NCL;
using namespace Rendering;

int constexpr MAX_FILE_SIZE = 512;

std::map<std::string, TextureLoadFunction> TextureLoader::fileHandlers;

bool TextureLoader::LoadTexture(const std::string& filename, char*& outData, int& width, int &height, int &channels, int&flags, bool resize) {
	if (filename.empty()) {
		std::cout << "fail1"+filename << std::endl;
		return false;
	}

	bool highestQuality = false;
	bool createTextureFiles = false;

	/*if(unsigned char* inData = LoadTxtrFile(filename, outData, &width, &height, &channels, &flags)) {
		outData = (char*)inData;
		return true;
	}*/

	std::string textureFilename = filename.substr(0, filename.find('.')) + ".texture";

	std::filesystem::path path(filename);
	
	std::string extension = path.extension().string();

	bool isAbsolute = path.is_absolute();

	auto it = fileHandlers.find(extension);

	std::string realPath = isAbsolute ? filename : Assets::TEXTUREDIR + filename;

	std::string texturePath = isAbsolute ? textureFilename : Assets::TEXTUREDIR + textureFilename;

	if (it != fileHandlers.end()) {
		//There's a custom handler function for this, just use that
		return it->second(realPath, outData, width, height, channels, flags);
	}

	stbi_uc* texData = nullptr;

	if(!highestQuality) {
		texData = stbi_load(texturePath.c_str(), &width, &height, &channels, 4);
	}

	if (!texData) {
		texData = stbi_load(realPath.c_str(), &width, &height, &channels, 4); //4 forces this to always be rgba!
		if(texData && createTextureFiles) CreateTextureFile(filename, texData, width, height, resize);
	}

	channels = 4; //it gets forced, we don't care about the 'real' channel size

	if (texData) {
		outData = (char*)texData;
		std::cout << "success"+filename << "\n";
		//CreateTxtrFile(filename, outData, width, height, channels, flags);
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

void TextureLoader::CreateTxtrFile(const std::string& filename, char*& outdata, int& width, int& height, int& channels, int& flags) {
	std::string txtrFilename = filename.substr(0, filename.find('.')) + ".txtr";

	std::filesystem::path path(txtrFilename);

	bool isAbsolute = path.is_absolute();

	std::string realPath = isAbsolute ? txtrFilename : Assets::TEXTUREDIR + txtrFilename;

	std::ofstream newfile(realPath, std::ios::binary);

	newfile << width << "," << height << "," << channels << "," << flags << ",\n";

	int i = 0;
	int count = 0;
	while(count < width) {
		newfile << outdata[i];
		i++;
		if(outdata[i] == '\0') count++;
	}

	newfile.close();
}

unsigned char* TextureLoader::LoadTxtrFile(const std::string& filename, char*& outData, int* width, int* height, int* channels, int* flags) {
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

		char* tempStr = new char[5];
		tempStr[4] = '\0';
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

		f.close();

		return returnData;
	}

	return nullptr;
}

void TextureLoader::CreateTextureFile(const std::string& filename, unsigned char* texData, int& width, int& height, bool resize) {
	stbi_uc* texture = nullptr;
	int newWidth, newHeight;
	bool resized = false;

	std::string texFilename = filename.substr(0, filename.find('.')) + ".texture";

	std::filesystem::path path(texFilename);

	bool isAbsolute = path.is_absolute();

	std::string realPath = isAbsolute ? texFilename : Assets::TEXTUREDIR + texFilename;

	if(width > MAX_FILE_SIZE && height > MAX_FILE_SIZE && resize) {
		resized = true;
		newWidth = width;
		newHeight = height;
		while (newWidth > MAX_FILE_SIZE && newHeight > MAX_FILE_SIZE) {
			newWidth = newWidth / 2;
			newHeight = newHeight / 2;
		}
		texture = (stbi_uc*)malloc(newWidth * newHeight * 4);
		stbir_resize_uint8(texData, width, height, 0, texture, newWidth, newHeight, 0, 4);
	}
	if (!texture) {
		texture = texData;
		newWidth = width;
		newHeight = height;
	}
	if (!stbi_write_png(realPath.c_str(), newWidth, newHeight, 4, texture, newWidth * 4)) {
		std::cout << "Failed to create .texture file for " << texFilename << "\n";
		return;
	}
	if (resized) {
		free(texture);
		texture = nullptr;
	}
}
