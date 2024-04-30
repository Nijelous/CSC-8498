#include "MshLoader.h"
#include "Assets.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include "Vector2i.h"
#include "Vector3i.h"
#include "Vector4i.h"

#include "Matrix4.h"

#include "Maths.h"

#include "Mesh.h"

using namespace NCL;
using namespace Rendering;
using namespace Maths;

bool MshLoader::LoadMesh(const std::string& filename, Mesh& destinationMesh) {
	
	std::ifstream file(Assets::MESHDIR + filename);

	std::string filetype;
	int fileVersion;

	file >> filetype;

	if (filetype != "MeshGeometry") {
		std::cout << __FUNCTION__ << " File is not a Mesh file!\n";
		return false;
	}

	file >> fileVersion;

	if (fileVersion != 1) {
		std::cout << __FUNCTION__ << " Mesh file has incompatible version!\n";
		return false;
	}

	int numMeshes = 0; //read
	int numVertices = 0; //read
	int numIndices = 0; //read
	int numChunks = 0; //read

	file >> numMeshes;
	file >> numVertices;
	file >> numIndices;
	file >> numChunks;

	for (int i = 0; i < numChunks; ++i) {
		int chunkType = (int)GeometryChunkTypes::VPositions;

		file >> chunkType;

		switch ((GeometryChunkTypes)chunkType) {
		case GeometryChunkTypes::VPositions: {
			vector<Vector3> positions;
			ReadTextFloats(file, positions);
			destinationMesh.SetVertexPositions(positions);
		}break;
		case GeometryChunkTypes::VColors: {
			vector<Vector4> colours;
			ReadTextFloats(file, colours);
			destinationMesh.SetVertexColours(colours);
		}break;
		case GeometryChunkTypes::VNormals: {
			vector<Vector3> normals;
			ReadTextFloats(file, normals);
			destinationMesh.SetVertexNormals(normals);
		}break;
		case GeometryChunkTypes::VTangents: {
			vector<Vector4> tangents;
			ReadTextFloats(file, tangents);
			destinationMesh.SetVertexTangents(tangents);

		}break;
		case GeometryChunkTypes::VTex0: {
			vector<Vector2> texCoords;
			ReadTextFloats(file, texCoords);
			destinationMesh.SetVertexTextureCoords(texCoords);

		}break;
		case GeometryChunkTypes::Indices: {
			vector<unsigned int> indices;
			ReadIntegers(file, indices);
			destinationMesh.SetVertexIndices(indices);
		}break;

		case GeometryChunkTypes::VWeightValues: {
			vector<Vector4> skinWeights;
			ReadTextFloats(file, skinWeights);
			destinationMesh.SetVertexSkinWeights(skinWeights);
		}break;
		case GeometryChunkTypes::VWeightIndices: {
			vector<Vector4i> skinIndices;
			ReadTextInts(file, skinIndices);
			destinationMesh.SetVertexSkinIndices(skinIndices);
		}break;
		case GeometryChunkTypes::JointNames: {
			std::vector<std::string> jointNames;
			ReadJointNames(file, jointNames);
			destinationMesh.SetJointNames(jointNames);
		}break;
		case GeometryChunkTypes::JointParents: {
			vector<int> parents;
			ReadJointParents(file, parents);
			destinationMesh.SetJointParents(parents);
		}break;
		case GeometryChunkTypes::BindPose: {
			vector<Matrix4> bindPose;
			ReadRigPose(file, bindPose);
			destinationMesh.SetBindPose(bindPose);
		}break;
		case GeometryChunkTypes::BindPoseInv: {
			vector<Matrix4> inverseBindPose;
			ReadRigPose(file, inverseBindPose);
			destinationMesh.SetInverseBindPose(inverseBindPose);
		}break;
		case GeometryChunkTypes::SubMeshes: {
			vector<SubMesh> subMeshes;
			ReadSubMeshes(file, subMeshes);

			destinationMesh.SetSubMeshes(subMeshes);
		}break;
		case GeometryChunkTypes::SubMeshNames: {
			std::vector<std::string> subMeshNames;
			ReadSubMeshNames(file, subMeshNames);
			destinationMesh.SetSubMeshNames(subMeshNames);
		}break;
		case GeometryChunkTypes::BindPoseIndices: {
			std::vector<int> bindPoseIndices;
			ReadIntegerArray(file, bindPoseIndices);
			destinationMesh.SetBindPoseIndices(bindPoseIndices);
		}break;//New!	

		case GeometryChunkTypes::BindPoseStates: {
			std::vector<Mesh::SubMeshPoses>	bindPoseStates;
			ReadBindposes(file, bindPoseStates);
			destinationMesh.SetBindPoseStates(bindPoseStates);
		}break;//New!

		}
	}
	
	destinationMesh.SetPrimitiveType(GeometryPrimitive::Triangles);

	return true;
}
void MshLoader::ReadIntegerArray(std::ifstream& file, vector<int>& into) {//New!
	int count = 0;
	file >> count;

	std::string line;
	file >> line;

	unsigned int temp;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	for (char c : line) {
		switch (c) {
		case ',':
			temp = StrToUInt(tempStr, strLen);
			into.emplace_back(temp);
			strLen = 0;
			break;
		default:
			tempStr[strLen] = c;
			strLen++;
			break;
		}
	}

	tempStr = nullptr;
	delete[] tempStr;
}

void MshLoader::ReadBindposes(std::ifstream& file, vector<Mesh::SubMeshPoses>& bindPoses) {//New!
	int poseCount = 0;
	file >> poseCount;

	std::string line;
	file >> line;

	Mesh::SubMeshPoses temp;
	int count = 0;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	for (char c : line) {
		switch (c) {
		case ',':
			count++;
			switch (count % 2) {
			case 0:
				temp.count = StrToUInt(tempStr, strLen);
				bindPoses.emplace_back(temp);
				break;
			case 1:
				temp.start = StrToUInt(tempStr, strLen);
				break;
			}
			strLen = 0;
			break;
		default:
			tempStr[strLen] = c;
			strLen++;
			break;
		}
	}
}

void MshLoader::ReadRigPose(std::ifstream& file, vector<Matrix4>& into) {
	int matCount = 0;
	file >> matCount;

	std::string line;
	file >> line;

	Matrix4 mat;
	int count = 0;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	char* tempExp = new char[4];
	tempExp[3] = '\0';
	int expLen = 0;
	bool hasExponent = false;

	for (char c : line) {
		switch (c) {
		case ',':
			mat.array[count / 4][count % 4] = StrToFloat(tempStr, tempExp, hasExponent, strLen);
			count++;
			if (count == 16) {
				into.emplace_back(mat);
				count = 0;
			}
			strLen = 0;
			expLen = 0;
			hasExponent = false;
			break;
		case 'E':
		case 'e':
			hasExponent = true;
			break;
		default:
			if (hasExponent) {
				if (expLen == 0) {
					if (c == '-') tempExp[expLen] = '-';
					else tempExp[expLen] = '+';
				}
				else tempExp[expLen] = c;
				expLen++;
			}
			else {
				tempStr[strLen] = c;
				strLen++;
			}
			break;
		}
	}
	tempStr = nullptr;
	delete[] tempStr;

	tempExp = nullptr;
	delete[] tempExp;
}

void MshLoader::ReadJointParents(std::ifstream& file, std::vector<int>& parentIDs) {
	int jointCount = 0;
	file >> jointCount;

	std::string line;
	file >> line;

	int temp;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	for (char c : line) {
		switch (c) {
		case ',':
			temp = StrToInt(tempStr, strLen);
			parentIDs.emplace_back(temp);
			strLen = 0;
			break;
		default:
			tempStr[strLen] = c;
			strLen++;
			break;
		}
	}
	tempStr = nullptr;
	delete[] tempStr;
}

void MshLoader::ReadJointNames(std::ifstream& file, std::vector<std::string>& jointNames) {
	int jointCount = 0;
	file >> jointCount;

	std::string line;
	file >> line;

	std::string jointName;
	int strLen = 0;

	for (char c : line) {
		switch (c) {
		case ',':
			jointNames.emplace_back(jointName);
			jointName = "";
			strLen = 0;
			break;
		default:
			jointName += c;
			strLen++;
			break;
		}
	}
}

void MshLoader::ReadSubMeshes(std::ifstream& file, std::vector<SubMesh>& subMeshes) {
	std::string line;
	file >> line;

	SubMesh temp;
	int count = 0;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	for (char c : line) {
		switch (c) {
		case ',':
			count++;
			switch (count % 2) {
			case 0:
				temp.count = StrToUInt(tempStr, strLen);
				subMeshes.emplace_back(temp);
				break;
			case 1:
				temp.start = StrToUInt(tempStr, strLen);
				break;
			}
			strLen = 0;
			break;
		default:
			tempStr[strLen] = c;
			strLen++;
			break;
		}
	}

	tempStr = nullptr;
	delete[] tempStr;
}

void MshLoader::ReadSubMeshNames(std::ifstream& file, std::vector<std::string>& subMeshNames) {
	std::string line;
	file >> line;

	std::string meshName;
	int strLen = 0;

	for (char c : line) {
		switch (c) {
		case ',':
			subMeshNames.emplace_back(meshName);
			meshName = "";
			strLen = 0;
			break;
		default:
			meshName += c;
			strLen++;
			break;
		}
	}
}

float MshLoader::StrToFloat(const char* str, const char* exp, const bool hasExponent, int strLen) {
	bool sign = false;
	bool decimal = false;
	bool exponentSign = false;
	int exponent = 0;
	double num = 0.0;
	double factor = 1.0;

	if (*str == '-') {
		sign = true;
		strLen--;
		str++;
	}

	for (; strLen > 0; strLen--) {
		if (*str == '.') {
			if (decimal) return -1;
			decimal = true;
			str++;
			continue;
		}

		if (decimal) factor *= 0.1;

		int digit = *str - '0';
		if (digit < 0 || digit > 9) return -1;
		if (decimal) num += factor * digit;
		else num = num * 10.0 + digit;
		str++;
	}

	if(hasExponent) {
		if(*exp == '-') {
			exponentSign = true;
		}
		exp++;
		exponent = (*exp - '0') * 10;
		exp++;
		exponent += *exp - '0';
		if (exponentSign) exponent = -exponent;
	}

	num = num * pow(10, exponent);

	if (sign) num = -num;
	return num;
}

unsigned int MshLoader::StrToUInt(const char* str, int strLen) {
	unsigned int num = 0;

	for (; strLen > 0; strLen--) {
		int digit = *str - '0';
		if (digit < 0 || digit > 9) return -1;
		num = num * 10 + digit;
		str++;
	}

	return num;
}

int MshLoader::StrToInt(const char* str, int strLen) {
	bool sign = false;
	int num = 0;

	if (*str == '-') {
		sign = true;
		strLen--;
		str++;
	}

	for (; strLen > 0; strLen--) {
		int digit = *str - '0';
		if (digit < 0 || digit > 9) return -1;
		num = num * 10 + digit;
		str++;
	}

	if (sign) num = -num;

	return num;
}

void* MshLoader::ReadVertexData(GeometryChunkData dataType, GeometryChunkTypes chunkType, int numVertices) {
	int numElements = 3;

	if (chunkType == GeometryChunkTypes::VTex0 ||
		chunkType == GeometryChunkTypes::VTex1) {
		numElements = 2;
	}
	else if (chunkType == GeometryChunkTypes::VColors) {
		numElements = 4;
	}

	int bytesPerElement = 4;

	if (dataType == GeometryChunkData::dShort) {
		bytesPerElement = 2;
	}
	else if (dataType == GeometryChunkData::dByte) {
		bytesPerElement = 1;
	}

	char* data = new char[numElements * bytesPerElement];

	return data;
}

void MshLoader::ReadTextInts(std::ifstream& file, vector<Vector2i>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector2i temp;
		file >> temp[0];
		file >> temp[1];
		element.emplace_back(temp);
	}
}

void MshLoader::ReadTeReadTextIntsxtFloats(std::ifstream& file, vector<Vector3i>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector3i temp;
		file >> temp[0];
		file >> temp[1];
		file >> temp[2];
		element.emplace_back(temp);
	}
}

void MshLoader::ReadTextInts(std::ifstream& file, vector<Vector4i>& element) {
	std::string line;
	file >> line;

	Vector4i temp;
	int count = 0;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	for (char c : line) {
		switch (c) {
		case ',':
			count++;
			switch(count % 4) {
			case 0:
				temp[3] = StrToUInt(tempStr, strLen);
				element.emplace_back(temp);
				break;
			case 1:
				temp[0] = StrToUInt(tempStr, strLen);
				break;
			case 2:
				temp[1] = StrToUInt(tempStr, strLen);
				break;
			case 3:
				temp[2] = StrToUInt(tempStr, strLen);
				break;
			}
			strLen = 0;
			break;
		default:
			tempStr[strLen] = c;
			strLen++;
			break;
		}
	}
	tempStr = nullptr;
	delete[] tempStr;
}

void MshLoader::ReadTextFloats(std::ifstream& file, vector<Vector2>& element) {
	std::string line;
	file >> line;

	Vector2 temp;
	int count = 0;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	char* tempExp = new char[4];
	tempExp[3] = '\0';
	int expLen = 0;
	bool hasExponent = false;

	for (char c : line) {
		switch (c) {
		case ',':
			count++;
			switch (count % 2) {
			case 0:
				temp.y = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				element.emplace_back(temp);
				break;
			case 1:
				temp.x = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				break;
			}
			strLen = 0;
			expLen = 0;
			hasExponent = false;
			break;
		case 'E':
		case 'e':
			hasExponent = true;
			break;
		default:
			if (hasExponent) {
				if (expLen == 0) {
					if (c == '-') tempExp[expLen] = '-';
					else tempExp[expLen] = '+';
				}
				else tempExp[expLen] = c;
				expLen++;
			}
			else {
				tempStr[strLen] = c;
				strLen++;
			}
			break;
		}
	}
	tempStr = nullptr;
	delete[] tempStr;

	tempExp = nullptr;
	delete[] tempExp;
}

void MshLoader::ReadTextFloats(std::ifstream& file, vector<Vector3>& element) {
	std::string line;
	file >> line;

	Vector3 temp;
	int count = 0;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;
	
	char* tempExp = new char[4];
	tempExp[3] = '\0';
	int expLen = 0;
	bool hasExponent = false;

	for(char c : line) {
		switch(c) {
		case ',':
			count++;
			switch(count % 3) {
			case 0:
				temp.z = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				element.emplace_back(temp);
				break;
			case 1:
				temp.x = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				break;
			case 2:
				temp.y = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				break;
			}
			strLen = 0;
			expLen = 0;
			hasExponent = false;
			break;
		case 'E':
		case 'e':
			hasExponent = true;
			break;
		default:
			if(hasExponent) {
				if (expLen == 0) {
					if (c == '-') tempExp[expLen] = '-';
					else tempExp[expLen] = '+';
				}
				else tempExp[expLen] = c;
				expLen++;
			}
			else {
				tempStr[strLen] = c;
				strLen++;
			}
			break;
		}
	}
	tempStr = nullptr;
	delete[] tempStr;

	tempExp = nullptr;
	delete[] tempExp;
}

void MshLoader::ReadTextFloats(std::ifstream& file, vector<Vector4>& element) {
	std::string line;
	file >> line;

	Vector4 temp;
	int count = 0;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	char* tempExp = new char[4];
	tempExp[3] = '\0';
	int expLen = 0;
	bool hasExponent = false;

	for (char c : line) {
		switch (c) {
		case ',':
			count++;
			switch (count % 4) {
			case 0:
				temp.w = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				element.emplace_back(temp);
				break;
			case 1:
				temp.x = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				break;
			case 2:
				temp.y = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				break;
			case 3:
				temp.z = StrToFloat(tempStr, tempExp, hasExponent, strLen);
				break;
			}
			strLen = 0;
			expLen = 0;
			hasExponent = false;
			break;
		case 'E':
		case 'e':
			hasExponent = true;
			break;
		default:
			if (hasExponent) {
				if (expLen == 0) {
					if (c == '-') tempExp[expLen] = '-';
					else tempExp[expLen] = '+';
				}
				else tempExp[expLen] = c;
				expLen++;
			}
			else {
				tempStr[strLen] = c;
				strLen++;
			}
			break;
		}
	}
	tempStr = nullptr;
	delete[] tempStr;

	tempExp = nullptr;
	delete[] tempExp;
}

void MshLoader::ReadIntegers(std::ifstream& file, vector<unsigned int>& elements) {
	std::string line;
	file >> line;

	unsigned int temp;

	char* tempStr = new char[14];
	tempStr[13] = '\0';
	int strLen = 0;

	for (char c : line) {
		switch (c) {
		case ',':
			temp = StrToUInt(tempStr, strLen);
			elements.emplace_back(temp);
			strLen = 0;
			break;
		default:
			tempStr[strLen] = c;
			strLen++;
			break;
		}
	}

	tempStr = nullptr;
	delete[] tempStr;
}