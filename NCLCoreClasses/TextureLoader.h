/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <map>
#include <functional>


namespace NCL {
	namespace Rendering {
		class Texture;
	}

	typedef std::function<bool(const std::string& filename, char*& outData, int& width, int &height, int &channels, int&flags)> TextureLoadFunction;

	typedef std::function<Rendering::Texture*(const std::string& filename)> APILoadFunction;

	class TextureLoader	{
	public:
		static bool LoadTexture(const std::string& filename, char*& outData, int& width, int &height, int &channels, int&flags);

		static void RegisterTextureLoadFunction(TextureLoadFunction f, const std::string&fileExtension);

		static void DeleteTextureData(char* data);
	protected:
		static unsigned int StrToUInt(const char* str, int strLen);

		static void CreateTxtrFile(const std::string& filename, char*& outdata, int& width, int& height, int& channels, int& flags);

		static unsigned char* LoadTxtrFile(const std::string& filename, char*& outData, int* width, int* height, int* channels, int* flags);

		static void CreateTextureFile(const std::string& filename, unsigned char* texData, int& width, int& height);

		static std::string GetFileExtension(const std::string& fileExtension);

		static std::map<std::string, TextureLoadFunction> fileHandlers;
	};
}

