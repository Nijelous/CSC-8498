#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "PlayerObject.h"
#include "CapsuleVolume.h"
#include "../CSC8503/InventoryBuffSystem/Item.h"
#include "Interactable.h"
#include "../CSC8503/LevelManager.h"

#include "Window.h"
#include "GameWorld.h"

using namespace NCL::CSC8503;

namespace {
	constexpr float STOPPING_SPEED = 3.f;

	constexpr float CHAR_STANDING_HEIGHT = 1.4f;
	constexpr float CHAR_CROUCH_HEIGHT = .7f;
	constexpr float CROUCH_OFFSET = 1;

	constexpr int MAX_CROUCH_SPEED = 5;
	constexpr int MAX_WALK_SPEED = 9;
	constexpr int MAX_SPRINT_SPEED = 20;

	constexpr int SPED_UP_MAX_CROUCH_SPEED = 7.5;
	constexpr int SPED_UP_MAX_WALK_SPEED = 13.5;
	constexpr int SPED_UP_MAX_SPRINT_SPEED = 30;

	constexpr int DEFAULT_CROUCH_SPEED = 35;
	constexpr int DEFAULT_WALK_SPEED = 40;
	constexpr int DEFAULT_SPRINT_SPEED = 50;

	//75% * Default speed, Rounded up
	constexpr int SLOWED_CROUCH_SPEED = 26;
	constexpr int SLOWED_WALK_SPEED = 30;
	constexpr int SLOWED_SPRINT_SPEED = 38;

	//150% * Default speed, Rounded up
	constexpr int SPED_UP_CROUCH_SPEED = 53;
	constexpr int SPED_UP_WALK_SPEED = 60;
	constexpr int SPED_UP_SPRINT_SPEED = 75;

	constexpr float WALK_ACCELERATING_SPEED = 1000.0f;
	constexpr float SPRINT_ACCELERATING_SPEED = 2000.0f;

	//150% * Default speed, Rounded up
	constexpr float SPED_UP_WALK_ACCELERATING_SPEED = 1500.0f;
	constexpr float SPED_UP_SPRINT_ACCELERATING_SPEED = 3000.0f;

	constexpr float LONG_INTERACT_WINDOW = 0.1f;
	constexpr float TIME_UNTIL_LONG_INTERACT = 1.5f;
	constexpr float TIME_UNTIL_PICKPOCKET = 0.75f;

	constexpr float MAX_PICKPOCKET_PITCH_DIFF = 20;

	constexpr bool DEBUG_MODE = true;
}

PlayerObject::PlayerObject(GameWorld* world, const std::string& objName,
	InventoryBuffSystem::InventoryBuffSystemClass* inventoryBuffSystemClassPtr,
	SuspicionSystem::SuspicionSystemClass* suspicionSystemClassPtr, PrisonDoor* prisonDoorPtr,
	int playerID,int walkSpeed, int sprintSpeed, int crouchSpeed, Vector3 boundingVolumeOffset) {
	mName = objName;
	mGameWorld = world;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
	mSuspicionSystemClassPtr = suspicionSystemClassPtr;
	mPrisonDoorPtr = prisonDoorPtr;
	mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(this);
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mWalkSpeed = walkSpeed;
	mSprintSpeed = sprintSpeed;
	mCrouchSpeed = crouchSpeed;
	mMovementSpeed = walkSpeed;
	mObjectState = Walk;
	mPlayerSpeedState = Default;
	mIsCrouched = false;
	mActiveItemSlot = 0;

	mPlayerID = playerID;
	mPlayerPoints = 0;
	mIsPlayer = true;
	mHasSilentSprintBuff = false;
	mInteractHeldDt = 0;
}

PlayerObject::~PlayerObject() {
	mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Detach(this);
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Detach(this);
}


void PlayerObject::UpdateObject(float dt) {
	if (mPlayerSpeedState != Stunned) {
		MovePlayer(dt);
		RayCastFromPlayer(mGameWorld, dt);
		if (mInventoryBuffSystemClassPtr != nullptr)
			ControlInventory();
		if (!Window::GetKeyboard()->KeyHeld(KeyCodes::E)) {
			mInteractHeldDt = 0;
		}
	}

	AttachCameraToPlayer(mGameWorld);

	float yawValue = mGameWorld->GetMainCamera().GetYaw();
	MatchCameraRotation(yawValue);

	if (mPlayerSpeedState == SpedUp)
		EnforceSpedUpMaxSpeeds();
	else
		EnforceMaxSpeeds();

	if (DEBUG_MODE)
	{
		Debug::Print("Sus:" + std::to_string(
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->GetLocalSusMetreValue(0)
		), Vector2(70, 90));
		if(mHasSilentSprintBuff)
			Debug::Print("HasSilentSprint", Vector2(70, 95));
		switch(mPlayerSpeedState){
		case SpedUp:
			Debug::Print("Sped Up", Vector2(45, 80));
			break;
		case SlowedDown:
			Debug::Print("Slowed Down", Vector2(45, 80));
			break;
		case Stunned:
			Debug::Print("Stunned", Vector2(45, 80));
			break;
		}
		const PlayerInventory::item equippedItem = GetEquippedItem();
		//Handle Equipped Item Log
		const std::string& itemName = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemName(equippedItem);
		Debug::Print(itemName, Vector2(10, 80));
		const std::string& usesLeft = "UsesLeft : " + to_string(mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUsesLeft(mPlayerID, mActiveItemSlot));
		Debug::Print(usesLeft, Vector2(10, 85));
		Debug::Print(to_string(mGameWorld->GetMainCamera().GetYaw()), Vector2(54, 90));
	}
}

void PlayerObject::UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo){
	if (mPlayerID != playerNo)
		return;

	switch (buffEvent) {
	case slowApplied:
		ChangeToSlowedSpeeds();
		break;
	case slowRemoved:
		ChangeToDefaultSpeeds();
		break;
	case speedApplied:
		ChangeToSpedUpSpeeds();
		break;
	case speedRemoved:
		ChangeToDefaultSpeeds();
		break;
	case stunApplied:
		ChangeToStunned();
		break;
	case stunRemoved:
		ChangeToDefaultSpeeds();
		break;
	case silentSprintApplied:
		mHasSilentSprintBuff = true;
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
		RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerSprint, mPlayerID);
		break;
	case silentSprintRemoved:
		mHasSilentSprintBuff = false;

		mObjectState = Idle;

		break;
	default:
		break;
	}
}

PlayerInventory::item NCL::CSC8503::PlayerObject::GetEquippedItem() {
	return mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemInInventorySlot(mPlayerID, mActiveItemSlot);
}

void PlayerObject::ClosePrisonDoor(){
	mPrisonDoorPtr->Close();
}

float PlayerObject::GetPlayerPitch()
{
	//Might need to change 
	return mGameWorld->GetMainCamera().GetPitch();
}

void PlayerObject::AttachCameraToPlayer(GameWorld* world) {
	Vector3 offset = GetTransform().GetPosition();
	offset.y += 3;
	world->GetMainCamera().SetPosition(offset);
}

void PlayerObject::MovePlayer(float dt) {
	Vector3 fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
	Vector3 rightAxis = mGameWorld->GetMainCamera().GetRightVector();
	bool isIdle = true;
	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)){
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);
		isIdle = false;
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)){
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);
		isIdle = false;
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)){
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);
		isIdle = false;
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)){
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);
		isIdle = false;
	}

	bool isSprinting = Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT);
	bool isCrouching = Window::GetKeyboard()->KeyPressed(KeyCodes::CONTROL);
	
	GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(mGameWorld->GetMainCamera().GetPitch(), mGameWorld->GetMainCamera().GetYaw(), 0));

	if (isIdle){
		if(mObjectState != Idle && mSuspicionSystemClassPtr != nullptr ) {
			mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
				RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerSprint, mPlayerID);
			mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
				RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);
		}
		if (mIsCrouched)
			mObjectState = IdleCrouch;
		else
			mObjectState = Idle;
	}
	else {
		ActivateSprint(isSprinting);
		if (mIsCrouched)
			mObjectState = Crouch;
	}

	ToggleCrouch(isCrouching);

	//std::cout << mObjectState << std::endl;

	StopSliding();
}

void PlayerObject::RayCastFromPlayer(GameWorld* world, float dt) {
	bool isRaycastTriggered = false;
	NCL::CSC8503::InteractType interactType;

	//TODO(erendgrmnc): not a best way to handle, need to refactor here later.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::E)) {
		isRaycastTriggered = true;
		interactType = NCL::CSC8503::InteractType::Use;
		mInteractHeldDt = 0;
	}
	else if (Window::GetKeyboard()->KeyHeld(KeyCodes::E)) {
		mInteractHeldDt += dt;
		Debug::Print(to_string(mInteractHeldDt), Vector2(40, 90));
		if (mInteractHeldDt >= TIME_UNTIL_PICKPOCKET - LONG_INTERACT_WINDOW &&
			mInteractHeldDt <= TIME_UNTIL_PICKPOCKET + LONG_INTERACT_WINDOW) {
			isRaycastTriggered = true;
			interactType = NCL::CSC8503::InteractType::PickPocket;
			if(DEBUG_MODE)
				Debug::Print("PickPocket window", Vector2(40, 85));
		}
		if (mInteractHeldDt >= TIME_UNTIL_LONG_INTERACT - LONG_INTERACT_WINDOW &&
			mInteractHeldDt <= TIME_UNTIL_LONG_INTERACT + LONG_INTERACT_WINDOW) {
			isRaycastTriggered = true;
			interactType = NCL::CSC8503::InteractType::LongUse;
			if (DEBUG_MODE)
				Debug::Print("LongUse", Vector2(40, 85));
		}
	}
	if (Window::GetMouse()->ButtonPressed(MouseButtons::Left) && GetEquippedItem() != PlayerInventory::item::none) {
		ItemUseType equippedItemUseType = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUseType(GetEquippedItem());
		if (equippedItemUseType != ItemUseType::NeedInteractableToUse)
			return;
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
				if (interactablePtr != nullptr && interactablePtr->CanBeInteractedWith(interactType)) {
					interactablePtr->Interact(interactType);
					if (interactType == ItemUse) {
						mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->UseItemInPlayerSlot(mPlayerID, mActiveItemSlot);
					}

					return;
				}
				if (interactType == PickPocket)
				{
					GameObject* otherPlayerObject = dynamic_cast<GameObject*>(objectHit);
					if (otherPlayerObject != nullptr && IsSeenByGameObject(otherPlayerObject)) {
						mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->
							TransferItemBetweenInventories(1,
								0, this->GetPlayerID());
					}
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
			mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->UseItemInPlayerSlot(mPlayerID, mActiveItemSlot);
		}
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->DropItemFromPlayer(mPlayerID, mActiveItemSlot);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && DEBUG_MODE) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->TransferItemBetweenInventories(mPlayerID,mActiveItemSlot,1);
	}


}

void PlayerObject::ToggleCrouch(bool crouchToggled) {
	if (crouchToggled && mObjectState == Crouch) {
		//Crouch -> Walk
		StartWalking();
		if(mSuspicionSystemClassPtr != nullptr)
			mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
			AddActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);
	}
	else if (crouchToggled && mObjectState == Walk) {
		//Walk -> Crouch
		StartCrouching(); 
		if (mSuspicionSystemClassPtr != nullptr)
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
			RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);
	}
	else if (crouchToggled && mObjectState == IdleCrouch) {
		//Crouch -> Idle
		ChangeCharacterSize(CHAR_STANDING_HEIGHT);
		mIsCrouched = false;
		if (mSuspicionSystemClassPtr != nullptr)
			mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
			AddActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);
	}
	else if (crouchToggled && mObjectState == Idle) {
		//Idle -> Crouch
		StartCrouching();
		if (mSuspicionSystemClassPtr != nullptr)
			mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
			RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);
	}
}

void PlayerObject::ActivateSprint(bool isSprinting) {
	if (isSprinting) {
		//Sprint->Sprint 
		StartSprinting();

	}
	else if (!mIsCrouched) {
		//Sprint->Walk
		StartWalking();
	}
}

void PlayerObject::StartWalking() {
	if (!(mObjectState == Walk)) {
		if (mSuspicionSystemClassPtr != nullptr) {
			if (mObjectState == Sprint)
				mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
					RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerSprint, mPlayerID);
			mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
				AddActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);
		}
		if (mObjectState == Crouch)
			if(mPlayerSpeedState == SpedUp)
				mMovementSpeed = SPED_UP_WALK_ACCELERATING_SPEED;
			else
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
		if (mSuspicionSystemClassPtr != nullptr) {
			if (mObjectState== Walk)
				mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
					RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);

			if (!mHasSilentSprintBuff)
 				mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
					AddActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerSprint, mPlayerID);
		}

		if (mPlayerSpeedState == SpedUp)
			mMovementSpeed = SPED_UP_SPRINT_ACCELERATING_SPEED;
		else
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
		if (mSuspicionSystemClassPtr != nullptr){
			if (mObjectState == Sprint)
				mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
				RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerSprint, mPlayerID);
			if (mObjectState == Walk)
				mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
				RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);
		}

		if (mObjectState == Walk)
			mObjectState = Crouch;
		if (mObjectState == Idle)
			mObjectState = IdleCrouch;

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

bool PlayerObject::IsSeenByGameObject(GameObject* otherGameObject){
	float thisPitch = GetTransform().GetOrientation().ToEuler().y;
	float otherPitch= otherGameObject->GetTransform().GetOrientation().ToEuler().y;

	float PitchDiff = abs(otherPitch-thisPitch);
	return PitchDiff <= MAX_PICKPOCKET_PITCH_DIFF;
}

void PlayerObject::EnforceSpedUpMaxSpeeds() {
	Vector3 velocityDirection = mPhysicsObject->GetLinearVelocity();
	velocityDirection.Normalise();

	switch (mObjectState) {
	case(Crouch):
		if (mPhysicsObject->GetLinearVelocity().Length() > SPED_UP_MAX_CROUCH_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * SPED_UP_MAX_CROUCH_SPEED);
		break;
	case(Walk):
		if (mPhysicsObject->GetLinearVelocity().Length() > SPED_UP_MAX_WALK_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * SPED_UP_MAX_WALK_SPEED);
		break;
	case(Sprint):
		if (mPhysicsObject->GetLinearVelocity().Length() > SPED_UP_MAX_SPRINT_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * SPED_UP_MAX_SPRINT_SPEED);
		break;
	}
}

void PlayerObject::ChangeToDefaultSpeeds(){
	mCrouchSpeed = DEFAULT_CROUCH_SPEED;
	mWalkSpeed = DEFAULT_WALK_SPEED;
	mSprintSpeed = DEFAULT_SPRINT_SPEED;

	mPlayerSpeedState = Default;

	mObjectState = Idle;

}

void PlayerObject::ChangeToSlowedSpeeds(){
	mCrouchSpeed = SLOWED_CROUCH_SPEED;
	mWalkSpeed = SLOWED_WALK_SPEED;
	mSprintSpeed = SLOWED_SPRINT_SPEED;

	mPlayerSpeedState = SlowedDown;

	mObjectState = Idle;

}

void PlayerObject::ChangeToSpedUpSpeeds(){
	mCrouchSpeed = SPED_UP_CROUCH_SPEED;
	mWalkSpeed = SPED_UP_WALK_SPEED;
	mSprintSpeed = SPED_UP_SPRINT_SPEED;

	mPlayerSpeedState = SpedUp;

	mObjectState = Idle;

}

void PlayerObject::ChangeToStunned(){
	mCrouchSpeed = 0;
	mWalkSpeed = 0;
	mSprintSpeed = 0;

	if (mSuspicionSystemClassPtr != nullptr) {
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
			RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerSprint, mPlayerID);
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
			RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerWalk, mPlayerID);
	}

	mPhysicsObject->SetLinearVelocity(Vector3(0,0,0));
	mPlayerSpeedState = Stunned;

	mObjectState = Idle;

}

void NCL::CSC8503::PlayerObject::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved) {
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
	case InventoryBuffSystem::stunItemUsed:
		//TODO(erendgrmnc): handle multiplayer
		//mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(PlayerBuffs::stun, playerNo);
		//TODO(erendgrnc): stun guards only in singleplayer
		LevelManager::GetLevelManager()->AddBuffToGuards(PlayerBuffs::stun);
		break;
	default:
		break;
	}
}

void PlayerObject::MatchCameraRotation(float yawValue) {
	Matrix4 yawRotation = Matrix4::Rotation(yawValue, Vector3(0, 1, 0));
	GetTransform().SetOrientation(yawRotation);
}

void PlayerObject::StopSliding() {
	if ((mPhysicsObject->GetLinearVelocity().Length() < STOPPING_SPEED) && (mPhysicsObject->GetForce() == Vector3(0, 0, 0))) {
		float fallingSpeed = mPhysicsObject->GetLinearVelocity().y;
		mPhysicsObject->SetLinearVelocity(Vector3(0, fallingSpeed, 0));
	}
}

