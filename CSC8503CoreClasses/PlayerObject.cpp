#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "PlayerObject.h"
#include "CapsuleVolume.h"
#include "../CSC8503/InventoryBuffSystem/Item.h"
#include "Interactable.h"

#include "Window.h"
#include "GameWorld.h"

using namespace NCL::CSC8503;

namespace {
	constexpr float CHAR_STANDING_HEIGHT = 1.4f;
	constexpr float CHAR_CROUCH_HEIGHT = .7f;
	constexpr float CROUCH_OFFSET = 1;

	constexpr int MAX_CROUCH_SPEED = 5;
	constexpr int MAX_WALK_SPEED = 9;
	constexpr int MAX_SPRINT_SPEED = 20;

	constexpr float WALK_ACCELERATING_SPEED = 1000.0f;
	constexpr float SPRINT_ACCELERATING_SPEED = 2000.0f;
}

PlayerObject::PlayerObject(GameWorld* world, const std::string& objName, InventoryBuffSystemClass* inventoryBuffSystemClassPtr, int playerID,
	int walkSpeed, int sprintSpeed, int crouchSpeed, Vector3 boundingVolumeOffset) {
	mName = objName;
	mGameWorld = world;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;

	mWalkSpeed = walkSpeed;
	mSprintSpeed = sprintSpeed;
	mCrouchSpeed = crouchSpeed;
	mMovementSpeed = walkSpeed;
	mIsCrouched = false;
	mActiveItemSlot = 0;

	mFirstInventorySlotUsageCount = 0;
	mSecondInventorySlotUsageCount = 0;

	mPlayerID = playerID;
	mPlayerPoints = 0;
	mIsPlayer = true;

	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
}

PlayerObject::~PlayerObject() {
	
}


void PlayerObject::UpdateObject(float dt) {
	MovePlayer(dt);
	RayCastFromPlayer(mGameWorld);
	//temp if
	if (mInventoryBuffSystemClassPtr != nullptr)
		ControlInventory();
	AttachCameraToPlayer(mGameWorld);

	float yawValue = mGameWorld->GetMainCamera().GetYaw();
	MatchCameraRotation(yawValue);

	EnforceMaxSpeeds();
}

PlayerInventory::item NCL::CSC8503::PlayerObject::GetEquippedItem() {
	return mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetPlayerItem(mPlayerID, mActiveItemSlot);
}

void PlayerObject::OnCollisionBegin(GameObject* otherObject) {

}

void PlayerObject::AttachCameraToPlayer(GameWorld* world) {
	Vector3 offset = GetTransform().GetPosition();
	offset.y += 3;
	world->GetMainCamera().SetPosition(offset);
}

void PlayerObject::MovePlayer(float dt) {
	Vector3 fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
	Vector3 rightAxis = mGameWorld->GetMainCamera().GetRightVector();

	if (Window::GetKeyboard()->KeyDown(KeyCodes::W))
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S))
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::A))
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::D))
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

	bool isSprinting = Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT);
	bool isCrouching = Window::GetKeyboard()->KeyPressed(KeyCodes::CONTROL);
	ActivateSprint(isSprinting);
	ToggleCrouch(isCrouching);

	StopSliding();
}

void NCL::CSC8503::PlayerObject::OnPlayerUseItem() {

	if (mActiveItemSlot == 0) {
		mFirstInventorySlotUsageCount++;
	}
	else {
		mSecondInventorySlotUsageCount++;
	}
	int itemUseCount = mActiveItemSlot == 0 ? mFirstInventorySlotUsageCount : mSecondInventorySlotUsageCount;
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->UseItemInPlayerSlot(mPlayerID, mActiveItemSlot, itemUseCount);
}

void PlayerObject::RayCastFromPlayer(GameWorld* world) {
	bool isRaycastTriggered = false;
	NCL::CSC8503::InteractType interactType;

	//TODO(erendgrmnc): not a best way to handle, need to refactor here later.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::E)) {
		isRaycastTriggered = true;
		interactType = NCL::CSC8503::InteractType::Use;
	}
	else if (Window::GetMouse()->ButtonPressed(MouseButtons::Left) && GetEquippedItem() != PlayerInventory::item::none) {
		isRaycastTriggered = true;
		interactType = NCL::CSC8503::InteractType::ItemUse;
	}

	if (isRaycastTriggered)
	{
		std::cout << "Ray fired" << std::endl;
		Ray ray = CollisionDetection::BuidRayFromCenterOfTheCamera(world->GetMainCamera());
		RayCollision closestCollision;

		if (world->Raycast(ray, closestCollision, true, this)) {
			auto* objectHit = (GameObject*)closestCollision.node;
			if (objectHit) {
				Vector3 objPos = objectHit->GetTransform().GetPosition();
				Vector3 playerPos = GetTransform().GetPosition();

				float distance = (objPos - playerPos).Length();

				if (distance > 10.f) {
					std::cout << "Nothing hit in range" << std::endl;
					return;
				}

				//Check if object is an item.
				Item* item = dynamic_cast<Item*>(objectHit);
				if (item != nullptr) {
					item->OnPlayerInteract(mPlayerID);
					return;
				}

				//Check if object is an interactable.
				Interactable* interactablePtr = dynamic_cast<Interactable*>(objectHit);
				if (interactablePtr != nullptr)
				{
					interactablePtr->Interact(interactType);
					if (interactType == ItemUse) {
						OnPlayerUseItem();
					}

					return;
				}

				std::cout << "Object hit " << objectHit->GetName() << std::endl;
			}
		}
	}
}

void PlayerObject::ControlInventory() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
		mActiveItemSlot = 0;

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
		mActiveItemSlot = 1;

	if (Window::GetMouse()->GetWheelMovement() > 0)
		mActiveItemSlot = (mActiveItemSlot + 1 < InventoryBuffSystem::MAX_INVENTORY_SLOTS)
		? mActiveItemSlot + 1 : 0;

	if (Window::GetMouse()->GetWheelMovement() < 0)
		mActiveItemSlot = (mActiveItemSlot > 0)
		? mActiveItemSlot - 1 : InventoryBuffSystem::MAX_INVENTORY_SLOTS - 1;

	PlayerInventory::item equippedItem = GetEquippedItem();

	if (Window::GetMouse()->ButtonPressed(MouseButtons::Left)) {

		ItemUseType equippedItemUseType = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUseType(equippedItem);
		if (equippedItemUseType == DirectUse) {
			OnPlayerUseItem();
		}
	}

	//Handle Equipped Item Log
	const std::string& itemName = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemName(equippedItem);
	Debug::Print(itemName, Vector2(10, 80));
}

void PlayerObject::ToggleCrouch(bool isCrouching) {
	if (isCrouching && mObjectState == Crouch)
		StartWalking();
	else if (isCrouching && mObjectState == Walk)
		StartCrouching();
}

void PlayerObject::ActivateSprint(bool isSprinting) {
	if (isSprinting)
		StartSprinting();
	else if (!mIsCrouched)
		StartWalking();
	else if (mIsCrouched)
		StartCrouching();
}

void PlayerObject::StartWalking() {
	if (!(mObjectState == Walk)) {
		if (mObjectState == Crouch)
			mMovementSpeed = WALK_ACCELERATING_SPEED;

		mObjectState = Walk;
		mIsCrouched = false;
		ChangeCharacterSize(CHAR_STANDING_HEIGHT);
	}
	else
		mMovementSpeed = mWalkSpeed;
}

void PlayerObject::StartSprinting() {
	if (!(mObjectState == Sprint)) {
		mMovementSpeed = SPRINT_ACCELERATING_SPEED;

		mObjectState = Sprint;
		mIsCrouched = false;

		ChangeCharacterSize(CHAR_STANDING_HEIGHT);
	}
	else
		mMovementSpeed = mSprintSpeed;
}

void PlayerObject::StartCrouching() {
	if (!(mObjectState == Crouch)) {
		mObjectState = Crouch;
		mIsCrouched = true;
		mMovementSpeed = mCrouchSpeed;

		ChangeCharacterSize(CHAR_CROUCH_HEIGHT);

		Vector3 temp = GetTransform().GetPosition();
		temp.y -= CROUCH_OFFSET;
		GetTransform().SetPosition(temp);
	}
}

void PlayerObject::ChangeCharacterSize(float newSize) {
	dynamic_cast<CapsuleVolume*>(mBoundingVolume)->SetHalfHeight(newSize);
}

void PlayerObject::EnforceMaxSpeeds() {
	Vector3 velocityDirection = mPhysicsObject->GetLinearVelocity();
	velocityDirection.Normalise();

	switch (mObjectState) {
	case(Crouch):
		if (mPhysicsObject->GetLinearVelocity().Length() > MAX_CROUCH_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * MAX_CROUCH_SPEED);
		break;
	case(Walk):
		if (mPhysicsObject->GetLinearVelocity().Length() > MAX_WALK_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * MAX_WALK_SPEED);
		break;
	case(Sprint):
		if (mPhysicsObject->GetLinearVelocity().Length() > MAX_SPRINT_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * MAX_SPRINT_SPEED);
		break;
	}
}

void NCL::CSC8503::PlayerObject::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved) {

	if (isItemRemoved) {
		ResetEquippedItemUsageCount(invSlot);
	}

	switch (invEvent)
	{
	case InventoryBuffSystem::flagDropped:
		break;
	case InventoryBuffSystem::disguiseItemUsed:
		break;
	case InventoryBuffSystem::soundEmitterUsed:
		break;
	case InventoryBuffSystem::screwdriverUsed:
		break;
	default:
		break;
	}
}

void PlayerObject::MatchCameraRotation(float yawValue) {
	Matrix4 yawRotation = Matrix4::Rotation(yawValue, Vector3(0, 1, 0));
	GetTransform().SetOrientation(yawRotation);
}

void NCL::CSC8503::PlayerObject::ResetEquippedItemUsageCount(int inventorySlot) {
	switch (inventorySlot) {
	case 0: {
		mFirstInventorySlotUsageCount = 0;
		break;
	}
	case 1: {
		mSecondInventorySlotUsageCount = 0;
		break;
	}
	default:
		break;
	}
}

void PlayerObject::StopSliding() {
	if ((mPhysicsObject->GetLinearVelocity().Length() < 1) && (mPhysicsObject->GetForce() == Vector3(0, 0, 0))) {
		float fallingSpeed = mPhysicsObject->GetLinearVelocity().y;
		mPhysicsObject->SetLinearVelocity(Vector3(0, fallingSpeed, 0));
	}
}

