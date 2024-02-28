#include "Vent.h"
#include "../CSC8503/LevelManager.h"
#include "PlayerObject.h"

using namespace NCL::CSC8503;

Vent::Vent() {
	mIsOpen = false;
	mConnectedVent = nullptr;
	mInteractable = true;
}

void Vent::ConnectVent(Vent* vent) {
	if (vent == nullptr) return;
	mConnectedVent = vent;
}

void Vent::HandleItemUse() {
	mIsOpen = true;
}

void Vent::HandlePlayerUse() {
	auto* localPlayer = (GameObject*)(LevelManager::GetLevelManager()->GetTempPlayer());
	Transform& playerTransform = localPlayer->GetTransform();
	const Vector3& playerPos = localPlayer->GetTransform().GetPosition();

	const Vector3& teleportPos = mConnectedVent->GetTransform().GetPosition();
	const Quaternion& teleportOrient = mConnectedVent->GetTransform().GetOrientation();
	const Vector3 newPlayerPos = teleportPos + (teleportOrient * Vector3(5, 0, 0));

	playerTransform.SetPosition(newPlayerPos);
	playerTransform.SetOrientation(teleportOrient);
	LevelManager::GetLevelManager()->GetGameWorld()->GetMainCamera().SetYaw(mTransform.GetOrientation().ToEuler().y);
	mIsOpen = false;
}

void Vent::Interact(InteractType interactType) {

	switch (interactType) {
	case Use:
		HandlePlayerUse();
		break;
	case ItemUse:
		HandleItemUse();
		break;
	default:
		break;
	}
}

bool Vent::CanBeInteractedWith(InteractType interactType){
	switch (interactType) {
	case Use:
		return mIsOpen;
		break;
	case ItemUse:
		return CanUseItem() && !mIsOpen;
		break;
	default:
		break;
	}
}

bool Vent::CanUseItem(){
	auto* localPlayer = LevelManager::GetLevelManager()->GetTempPlayer();
	PlayerInventory::item usedItem = localPlayer->GetEquippedItem();

	switch (usedItem) {
	case InventoryBuffSystem::PlayerInventory::screwdriver:
		return true;
		break;
	default:
		break;
	}
}
