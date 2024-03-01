#include "UISystem.h"
#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"


using namespace NCL;
using namespace CSC8503;
using namespace InventoryBuffSystem;

namespace {


}

UISystem::UISystem() {

}

UISystem::~UISystem() {

}


UISystem::Icon* UISystem::AddIcon(Vector2 Pos, int horiSize, int vertSize, Texture* tex, bool isShown) {
	Icon* mIcon = new Icon();
	mIcon->mPosition = Pos;
	mIcon->mLength = horiSize;
	mIcon->mHeight = vertSize;
	mIcon->mTexture = tex;
	mIcon->mTransparency = isShown;
	mIconsVec.emplace_back(mIcon);
	return mIcon;
}

UISystem::Icon* UISystem::AddIcon(Icon* icon, bool isShown) {
	mIconsVec.emplace_back(icon);
	return icon;
}


void UISystem::SetIconPosition(Vector2 newPos, Icon& icon) {
	icon.mPosition = newPos;
}


std::vector<UISystem::Icon*>& UISystem::GetIcons() {
	return mIconsVec;
}

void UISystem::DeleteIcon(Icon icon) {
	if (mIconsVec.size() <= 0) {
		return;
	}
	int j = 0;
	for (auto& i : mIconsVec) {
		if (i->mPosition == icon.mPosition) {
			mIconsVec.erase(mIconsVec.begin() + j);
		}
		j++;
	}
}

void UISystem::BuildVerticesForIcon(const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords) {
	positions.reserve(positions.size() + 6);
	texCoords.reserve(texCoords.size() + 6);

	positions.emplace_back(Vector3(iconPos.x, iconPos.y + vertSize, 0));
	positions.emplace_back(Vector3(iconPos.x, iconPos.y, 0));
	positions.emplace_back(Vector3(iconPos.x + horiSize, iconPos.y + vertSize, 0));


	positions.emplace_back(Vector3(iconPos.x, iconPos.y, 0));
	positions.emplace_back(Vector3(iconPos.x + horiSize, iconPos.y + vertSize, 0));
	positions.emplace_back(Vector3(iconPos.x + horiSize, iconPos.y, 0));

	texCoords.emplace_back(Vector2(0, 1));
	texCoords.emplace_back(Vector2(0, 0));
	texCoords.emplace_back(Vector2(1, 1));

	texCoords.emplace_back(Vector2(0, 0));
	texCoords.emplace_back(Vector2(1, 1));
	texCoords.emplace_back(Vector2(1, 0));
}


void UISystem::ChangeEquipmentSlotTexture(int slotNum, Texture& texture) {


	switch (slotNum) {
	case FIRST_ITEM_SLOT:
		mFirstEquippedItem->mTexture = &texture;

		break;
	case SECOND_ITEM_SLOT:
		mSecondEquippedItem->mTexture = &texture;
		break;
	default:
		break;
	}
}

void UISystem::ChangeBuffSlotTransparency(int slotNum, bool isShown){
	switch (slotNum)
	{
	case SILENT_BUFF_SLOT:
		mSilentSprintIcon->mTransparency = isShown;
		break;
	case SLOW_BUFF_SLOT:
		mSlowIcon->mTransparency = isShown;
		break;
	case STUN_BUFF_SLOT:
		mStunIcon->mTransparency = isShown;
		break;
	case SPEED_BUFF_SLOT:
		mSpeedIcon->mTransparency = isShown;
		break;
	case SUSPISION_BAR_SLOT:
		mSuspensionBarIcon->mTransparency = isShown;
		break;
	case SUSPISION_INDICATOR_SLOT:
		mSuspensionIndicatorIcon->mTransparency = isShown;
		break;

	default:
		break;
	}

}

void UISystem::SetEquippedItemIcon(int slotNum, Icon& icon) {
	switch (slotNum) {
		case FIRST_ITEM_SLOT:
			mFirstEquippedItem = &icon;
			break;
		case SECOND_ITEM_SLOT:
			mSecondEquippedItem = &icon;
			break;
			break;
		case SILENT_BUFF_SLOT:
			mSilentSprintIcon = &icon;
			break;
		case SLOW_BUFF_SLOT:
			mSlowIcon = &icon;
			break;
		case STUN_BUFF_SLOT:
			mStunIcon = &icon;
			break;
		case SPEED_BUFF_SLOT:
			mSpeedIcon = &icon;
			break;
		case SUSPISION_BAR_SLOT:
			mSuspensionBarIcon = &icon;
			break;
		case SUSPISION_INDICATOR_SLOT:
			mSuspensionIndicatorIcon = &icon;
			break;

	default:
		break;
	}
}
