#include "Level.h"
#include "Vent.h"
#include "GameObject.h"
#include "BaseLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include <fstream>

using namespace NCL::CSC8503;

Level::Level(std::string levelPath) {
	mLevelName = levelPath.substr(24, levelPath.size()-29);
	mPlayerStartPositions = new Vector3[4];
	std::ifstream levelFile(levelPath);
	std::string line;
	getline(levelFile, line);

	std::string output = "";
	std::string currentKey = "";

	int indents = 0;
	int maxIndents = 0;
	bool writingKey = false;
	bool writingValue = false;

	std::vector<std::map<std::string, float>> keyValuePairs = std::vector<std::map<std::string, float>>();
	for (char c : line) {
		switch (c) {
		case '"':
			writingKey = !writingKey;
			currentKey = output;
			WriteValue(!writingKey, &keyValuePairs, output, &output, indents, maxIndents);
			if (writingKey && indents == 1) {
				maxIndents = 1;
			}
			break;
		case '{':
			indents++;
			if (indents > maxIndents) maxIndents++;
			writingValue = false;
			keyValuePairs.push_back(std::map<std::string, float>());
			break;
		case '}':
			WriteValue(writingValue, &keyValuePairs, currentKey, &output, indents, maxIndents);
			writingValue = false;
			indents--;
			break;
		case ',':
			WriteValue(writingValue, &keyValuePairs, currentKey, &output, indents, maxIndents);
			writingValue = false;
			if (indents == 1) {
				WriteVariable(keyValuePairs);
				while (keyValuePairs.size() > 1) {
					keyValuePairs.pop_back();
				}
			}
			break;
		case ':':
			writingValue = true;
			break;
		case '[':
		case ']':
			break;
		default:
			output += c;
			break;
		}
	}
	WriteVariable(keyValuePairs);
	for (int i = 0; i < mVentConnections.size(); i++) {
		mVents[i]->ConnectVent(mVents[mVentConnections[i]]);
	}
}

void Level::WriteValue(bool writingValue, std::vector<std::map<std::string, float>>* keyValuePairs, std::string key, std::string* value, int indents, int maxIndents) {
	if (writingValue) {
		if (keyValuePairs->size() <= maxIndents || indents < maxIndents) {
			(*keyValuePairs)[indents - 1][key] = std::atof(value->c_str());
		}
		else {
			(*keyValuePairs)[keyValuePairs->size() - 1][key] = std::atof(value->c_str());
		}
		*value = "";
	}
}

void Level::WriteVariable(std::vector<std::map<std::string, float>>& keyValuePairs) {
	switch (keyValuePairs[0].size()) {
	case 1:
		mTileMap[Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])] = (TileType)keyValuePairs[1]["type"];
		break;
	case 2:
		mRoomList[Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])] = Room((int)keyValuePairs[1]["type"]);
		break;
	case 3:
		mGuardCount = (int)keyValuePairs[0]["guardCount"];
		break;
	case 4:
		mCCTVCount = (int)keyValuePairs[0]["cctvCount"];
		break;
	case 5:
		mGuardPaths.push_back(std::vector<Vector3>());
		for (int i = 2; i < keyValuePairs.size(); i++) {
			mGuardPaths[mGuardPaths.size()-1].push_back(Vector3(keyValuePairs[i]["x"], keyValuePairs[i]["y"], -keyValuePairs[i]["z"]));
		}
		break;
	case 6:
		mCCTVTransforms.push_back(Matrix4::Rotation(keyValuePairs[3]["y"], Vector3(0, 1, 0)) *
			Matrix4::Translation(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])) * Matrix4::Scale(Vector3(1, 1, 1)) * Matrix4());
		break;
	case 7:
		mPrisonPosition = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
		break;
	case 8:
		for (int i = 0; i < 4; i++) {
			if (!mPlayerStartPositions[i].x) {
				mPlayerStartPositions[i] = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
				break;
			}
		}
		break;
	case 9:
		mLights.push_back((Light*)new DirectionLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"])));
		break;
	case 10:
		mLights.push_back((Light*)new PointLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]), keyValuePairs[1]["radius"]));
		break;
	case 11:
		mLights.push_back((Light*)new SpotLight(Vector3(keyValuePairs[4]["x"], keyValuePairs[4]["y"], -keyValuePairs[4]["z"]),
			Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]), 
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]), 
			keyValuePairs[1]["radius"], keyValuePairs[1]["angle"], 1.0f));
		break;
	case 12:
		mItemPositions.push_back(Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]));
		break;
	case 13:
		Vent* vent = new Vent();
		vent->GetTransform().SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
			.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"], -keyValuePairs[3]["z"]));
		mVents.push_back(vent);
		mVentConnections.push_back(keyValuePairs[1]["connectedVentID"]);
		break;
	}
}
