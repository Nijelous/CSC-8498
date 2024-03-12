﻿#ifdef USEGL
#pragma once
#include <functional>
#include "NetworkBase.h"
#include "GameSceneManager.h"
#include "NetworkedGame.h"

namespace NCL::CSC8503
{
	struct SyncObjectStatePacket;
}

namespace NCL::CSC8503 {
	struct SyncInteractablePacket;
	struct ClientSyncItemSlotPacket;
}

namespace NCL{
    namespace CSC8503{
        struct DeltaPacket;
        class GameServer;
        class GameClient;
        class NetworkPlayer;

        struct FullPacket;
        struct ClientPlayerInputPacket;
        struct ClientSyncBuffPacket;
        struct AddPlayerScorePacket;
        struct ClientSyncLocalActiveSusCausePacket;
        struct ClientSyncLocalSusChangePacket;
        struct ClientSyncGlobalSusChangePacket;
        struct ClientSyncLocationActiveSusCausePacket;
        struct ClientSyncLocationSusChangePacket;

        class DebugNetworkedGame : public NetworkedGame{
        public:
            DebugNetworkedGame();
            ~DebugNetworkedGame();
            
            bool GetIsServer() const;
            bool PlayerWonGame() override;
            bool PlayerLostGame() override;
            const bool GetIsGameStarted() const;

            const int GetClientLastFullID() const;

            bool StartAsServer();
            bool StartAsClient(char a, char b, char c, char d);

            void UpdateGame(float dt) override;

            void SetIsGameStarted(bool isGameStarted);
            void SetIsGameFinished(bool isGameFinished, int winningPlayerId);
            void StartLevel();

            void AddEventOnGameStarts(std::function<void()> event);

            void ReceivePacket(int type, GamePacket* payload, int source) override;
            void InitInGameMenuManager() override;

            void SendInteractablePacket(int networkObjectId, bool isOpen, int interactableItemType) const;
            void SendClientSyncItemSlotPacket(int playerNo, int invSlot, int inItem, int usageCount) const;
            void SendClientSyncBuffPacket(int playerNo, int buffType, bool toApply) const;
            void SendObjectStatePacket(int networkId, int state) const;
            void ClearNetworkGame();

            void SendClientSyncLocalActiveSusCausePacket(int playerNo, int activeSusCause, bool toApply) const;
            void SendClientSyncLocalSusChangePacket(int playerNo, int changedValue) const;
            void SendClientSyncGlobalSusChangePacket(int changedValue) const;
            void SendClientSyncLocationActiveSusCausePacket(int cantorPairedLocation, int activeSusCause, bool toApply) const;
            void SendClientSyncLocationSusChangePacket(int cantorPairedLocation, int changedValue) const;
            GameClient* GetClient() const;
            GameServer* GetServer() const;
            NetworkPlayer* GetLocalPlayer() const;

        protected:
            bool mIsGameStarted = false;
            bool mIsGameFinished = false;
            bool mIsServer = false;

            int mWinningPlayerId;
            int mLocalPlayerId;

            void UpdateAsServer(float dt);
            void UpdateAsClient(float dt);

            void BroadcastSnapshot(bool deltaFrame);
            void UpdateMinimumState();
            int GetPlayerPeerID(int peerId = -2);

            void SendStartGameStatusPacket();
            void SendFinishGameStatusPacket();
            
            void InitWorld() override;

            void HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID);

            void SpawnPlayers();

        	NetworkPlayer* AddPlayerObject(const Vector3& position, int playerNum);

            void HandleFullPacket(FullPacket* fullPacket);

            void HandleDeltaPacket(DeltaPacket* deltaPacket);

            void HandleClientPlayerInputPacket(ClientPlayerInputPacket* clientPlayerInputPacket, int playerPeerId);

            void HandleAddPlayerScorePacket(AddPlayerScorePacket* packet);

            void SyncPlayerList();

        	void SetItemsLeftToZero() override;

            void HandlePlayerEquippedItemChange(ClientSyncItemSlotPacket* packet) const;

            void HandleInteractablePacket(SyncInteractablePacket* packet) const;

        	void HandlePlayerBuffChange(ClientSyncBuffPacket* packet) const;

            void HandleObjectStatePacket(SyncObjectStatePacket* packet) const;

            void HandleLocalActiveSusCauseChange(ClientSyncLocalActiveSusCausePacket* packet) const;
            void HandleLocalSusChange(ClientSyncLocalSusChangePacket* packet) const;
            void HandleGlobalSusChange(ClientSyncGlobalSusChangePacket* packet) const;
            void HandleLocationActiveSusCauseChange(ClientSyncLocationActiveSusCausePacket* packet) const;
            void HandleLocationSusChange(ClientSyncLocationSusChangePacket* packet) const;
            std::vector<std::function<void()>> mOnGameStarts;

            int mNetworkObjectCache = 10;

            int mClientSideLastFullID;
            int mServerSideLastFullID;

        private:
        };
    }
}
#endif