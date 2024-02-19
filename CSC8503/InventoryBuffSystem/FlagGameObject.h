#pragma once
#include "GameObject.h"
#include "Vector3.h"
#include "StateMachine.h"
#include "InventoryBuffSystem.h"
#include "Item.h"
using namespace InventoryBuffSystem;

namespace NCL {
    namespace CSC8503 {
        class FlagGameObject : public Item {
        public:
            FlagGameObject(InventoryBuffSystemClass* inventoryBuffSystemClassPtr, std::map<GameObject*, int>* playerObjectToPlayerNoMap = nullptr);
            ~FlagGameObject();

            void GetFlag(int playerNo);
            void Reset();

            void OnPlayerInteract(int playerId = 0) override;
   
            virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) override;
            virtual void OnCollisionBegin(GameObject* otherObject) override;

        protected:
            std::map<GameObject*, int>* mPlayerObjectToPlayerNoMap;
            InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;
        };
    }
}
