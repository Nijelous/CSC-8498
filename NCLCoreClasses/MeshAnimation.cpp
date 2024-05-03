#include "MeshAnimation.h"
#include "Matrix4.h"
#include "Assets.h"

using namespace NCL;
using namespace Rendering;
using namespace Maths;

MeshAnimation::MeshAnimation() {
	jointCount	= 0;
	frameCount	= 0;
	frameRate	= 0.0f;
}

MeshAnimation::MeshAnimation(size_t jointCount, size_t frameCount, float frameRate, std::vector<Matrix4>& frames) {
	this->jointCount = jointCount;
	this->frameCount = frameCount;
	this->frameRate  = frameRate;
	this->allJoints  = frames;
}

MeshAnimation::MeshAnimation(const std::string& filename) : MeshAnimation() {
	std::ifstream file(Assets::MESHDIR + filename);

	std::string filetype;
	int fileVersion;

	file >> filetype;

	if (filetype != "MeshAnim") {
		std::cout << __FUNCTION__ << " File is not a MeshAnim file!\n";
		return;
	}
	file >> fileVersion;
	file >> frameCount;
	file >> jointCount;
	file >> frameRate;

	allJoints.reserve((size_t)frameCount * jointCount);

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
				allJoints.emplace_back(mat);
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

float MeshAnimation::StrToFloat(const char* str, const char* exp, const bool hasExponent, int strLen) {
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

	if (hasExponent) {
		if (*exp == '-') {
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

MeshAnimation::~MeshAnimation() {

}

const Matrix4* MeshAnimation::GetJointData(size_t frame) const {
	if (frame >= frameCount) {
		return nullptr;
	}
	int matStart = frame * jointCount;

	Matrix4* dataStart = (Matrix4*)allJoints.data();

	return dataStart + matStart;
}