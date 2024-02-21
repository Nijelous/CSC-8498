#pragma once

#include "GameObject.h"
#include "../CSC8503/InventoryBuffSystem/InventoryBuffSystem.h"

using namespace InventoryBuffSystem;

namespace NCL {
	namespace CSC8503 {
		class GameWorld;


		enum PlayerState {
			Walk,
			Sprint,
			Crouch
		};

		class PlayerObject : public GameObject, public PlayerInventoryObserver {
		public:
			PlayerObject(GameWorld* world, const std::string& objName = "", InventoryBuffSystemClass* inventoryBuffSystemClassPtr = nullptr, int playerID = 0,
				int walkSpeed = 40, int sprintSpeed = 50, int crouchSpeed = 35, Vector3 offset = Vector3(0, 0, 0));
			~PlayerObject();

			virtual void UpdateObject(float dt);

			PlayerInventory::item GetEquippedItem();

		protected:
			bool mIsCrouched;

			int mMovementSpeed;
			int mWalkSpeed;
			int mSprintSpeed;
			int mCrouchSpeed;
			int mActiveItemSlot;

			int mFirstInventorySlotUsageCount;
			int mSecondInventorySlotUsageCount;

			int mPlayerID;

			PlayerState mPlayerState;

			GameWorld* mGameWorld;
			InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;

			virtual void MovePlayer(float dt);
			
			void RayCastFromPlayer(GameWorld* world);

			void ControlInventory();

			void AttachCameraToPlayer(GameWorld* world);

			virtual void MatchCameraRotation(float yawValue);

			void ResetEquippedItemUsageCount(int inventorySlot);

			void StopSliding();

			void	ToggleCrouch(bool isCrouching);

			void	ActivateSprint(bool isSprinting);

			void	StartWalking();

			void	StartSprinting();

			void	StartCrouching();

			void	ChangeCharacterSize(float newSize);

			void	EnforceMaxSpeeds();

			void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override;
		private:
		};
	}
}

