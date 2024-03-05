﻿#include "DebugNetworkedGame.h"

#include <iostream>
#include <string>

#include "GameServer.h"
#include "GameClient.h"
#include "NetworkObject.h"
#include "NetworkPlayer.h"
#include "RenderObject.h"
#include "../CSC8503/InventoryBuffSystem/InventoryBuffSystem.h"
#include "../CSC8503/SuspicionSystem/SuspicionSystem.h"

namespace{
    constexpr int MAX_PLAYER = 4;
    constexpr int LEVEL_NUM = 0;
    
    constexpr const char* PLAYER_PREFIX = "Player";
    
}

DebugNetworkedGame::DebugNetworkedGame() {
    mThisServer = nullptr;
    mThisClient = nullptr;

    mClientSideLastFullID = 0;
    mServerSideLastFullID = 0;
    mGameState = GameSceneState::MainMenuState;
    
    NetworkBase::Initialise();
    mTimeToNextPacket = 0.0f;
    mPacketsToSnapshot = 0;

    for (int i = 0; i < MAX_PLAYER; i++){
        mPlayerList.push_back(-1);
    }
}

DebugNetworkedGame::~DebugNetworkedGame(){
}

bool DebugNetworkedGame::GetIsServer() const{
    return mIsServer;
}

void DebugNetworkedGame::StartAsServer(){
    mThisServer = new GameServer(NetworkBase::GetDefaultPort(), MAX_PLAYER);
    mIsServer = true;
    
    mThisServer->RegisterPacketHandler(Received_State, this);
    mThisServer->RegisterPacketHandler(String_Message, this);
    mThisServer->RegisterPacketHandler(BasicNetworkMessages::ClientPlayerInputState, this);
}

void DebugNetworkedGame::StartAsClient(char a, char b, char c, char d){
    mThisClient = new GameClient();
    mThisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

    mThisClient->RegisterPacketHandler(Delta_State, this);
    mThisClient->RegisterPacketHandler(Full_State, this);
    mThisClient->RegisterPacketHandler(Player_Connected, this);
    mThisClient->RegisterPacketHandler(Player_Disconnected, this);
    mThisClient->RegisterPacketHandler(String_Message, this);
    mThisClient->RegisterPacketHandler(GameStartState, this);
    mThisClient->RegisterPacketHandler(BasicNetworkMessages::SyncPlayers, this);
    mThisClient->RegisterPacketHandler(BasicNetworkMessages::GameEndState,this);
    mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncItemSlotUsage,this);
    mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncItemSlot,this);
    mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncBuffs, this);
}

void DebugNetworkedGame::UpdateGame(float dt){
    mTimeToNextPacket -= dt;
    if (mTimeToNextPacket < 0){
        if (mThisServer){
            UpdateAsServer(dt);
        }
        else if (mThisClient){
            UpdateAsClient(dt);
        }
        mTimeToNextPacket += 1.0f / 20.0f; //20hz server/client update

        if (mThisServer){
            SyncPlayerList();
        }
    }
    
    if (mIsGameStarted){
        //TODO(erendgrmnc): rewrite this logic after end-game conditions are decided.

        mLevelManager->GetGameWorld()->GetMainCamera().UpdateCamera(dt);
        if (mIsGameFinished){
            Debug::Print("Game Finished.", Vector2(5, 95), Debug::MAGENTA);
            mLevelManager->GetRenderer()->Render();
            return;
        }
        
        //DEBUG END GAME
        if (mThisServer){
            Debug::Print("SERVER", Vector2(5, 10), Debug::MAGENTA);
            if (Window::GetKeyboard()->KeyPressed(KeyCodes::R)){
                SetIsGameFinished(true);
            }
        }
        else{
            Debug::Print("CLIENT", Vector2(5, 10), Debug::MAGENTA);
        }
        
        mLevelManager->Update(dt, mGameState == InitialisingLevelState, false);
    }
    else{
        if (mThisServer){
            Debug::Print(" Waiting for player to join ...", Vector2(5, 95), Debug::RED);
            if (Window::GetKeyboard()->KeyPressed(KeyCodes::S)){
                SetIsGameStarted(true);
            }
        }
        else{
            Debug::Print(" Waiting for server to start ...", Vector2(5, 95), Debug::RED);
        }
        mLevelManager->GetRenderer()->Render();
    }

    if (mThisServer){
        mThisServer->UpdateServer();
    }
    if (mThisClient){
        mThisClient->UpdateClient();
    }
}

void DebugNetworkedGame::SetIsGameStarted(bool isGameStarted){
    this->mIsGameStarted = isGameStarted;
    if (mThisServer){
        SendStartGameStatusPacket();
    }
    if (isGameStarted){
        mGameState = GameSceneState::InitialisingLevelState;
        StartLevel();
    }
}

void DebugNetworkedGame::SetIsGameFinished(bool isGameFinished){
    mIsGameFinished = isGameFinished;
    if (mThisServer){
        SendFinishGameStatusPacket();
    }
}

void DebugNetworkedGame::StartLevel(){
    InitWorld();
    Debug::Print("Game Started", Vector2(10, 5));

    for (auto& event : mOnGameStarts){
        event();
    }
}

void DebugNetworkedGame::AddEventOnGameStarts(std::function<void()> event){
    mOnGameStarts.push_back(event);
}

void DebugNetworkedGame::ReceivePacket(int type, GamePacket* payload, int source){
    switch (type){
    case BasicNetworkMessages::String_Message: {
        int a = 0;
        break;
    }
    case BasicNetworkMessages::GameStartState: {
        GameStartStatePacket* packet = (GameStartStatePacket*)payload;
        SetIsGameStarted(packet->isGameStarted);
        break;
    }
    case BasicNetworkMessages::Full_State: {
        FullPacket* packet = (FullPacket*)payload;
        HandleFullPacket(packet);
        break;
    }
    case BasicNetworkMessages::Delta_State: {
        DeltaPacket* deltaPacket = (DeltaPacket*)payload;
        HandleDeltaPacket(deltaPacket);
        break;
    }
    case BasicNetworkMessages::GameEndState:{
        GameEndStatePacket* packet = (GameEndStatePacket*)payload;
        SetIsGameFinished(packet->isGameEnded);
        break;
    }
    case BasicNetworkMessages::SyncPlayers:{
        SyncPlayerListPacket* packet = (SyncPlayerListPacket*)payload;
        packet->SyncPlayerList(mPlayerList);
        break;
    }
    case  BasicNetworkMessages::ClientPlayerInputState: {
        ClientPlayerInputPacket* packet = (ClientPlayerInputPacket*)payload;
        HandleClientPlayerInputPacket(packet, source + 1);
        break;
    }
    case BasicNetworkMessages::ClientSyncItemSlot:{
        ClientSyncItemSlotPacket* packet = (ClientSyncItemSlotPacket*)(payload);
        HandlePlayerEquippedItemChange(packet);
        break;
    }
    case BasicNetworkMessages::ClientSyncBuffs: {
        ClientSyncBuffPacket* packet = (ClientSyncBuffPacket*)(payload);
        HandlePlayerBuffChange(packet);
        break;
    }
    case BasicNetworkMessages::ClientSyncLocalActiveCause: {
        ClientSyncLocalActiveSusCausePacket* packet = (ClientSyncLocalActiveSusCausePacket*)(payload);
        HandleLocalActiveSusCauseChange(packet);
        break;
    }
    case BasicNetworkMessages::ClientSyncLocalSusChange: {
        ClientSyncLocalSusChangePacket* packet = (ClientSyncLocalSusChangePacket*)(payload);
        HandleLocalSusChange(packet);
        break;
    }
    default:
        std::cout << "Received unknown packet. Type: " << payload->type  << std::endl;
        break;
    }
}

void DebugNetworkedGame::SendClientSyncItemSlotPacket(int playerNo, int invSlot, int inItem, int usageCount) const {
    PlayerInventory::item itemToEquip = (PlayerInventory::item)(inItem);
    NCL::CSC8503::ClientSyncItemSlotPacket packet(playerNo, invSlot, itemToEquip, usageCount);
    mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncBuffPacket(int playerNo, int buffType, bool toApply) const {
    PlayerBuffs::buff buffToSync = (PlayerBuffs::buff)(buffType);
    NCL::CSC8503::ClientSyncBuffPacket packet(playerNo, buffToSync, toApply);
    mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncLocalActiveSusCausePacket(int playerNo, int buffType, bool toApply) const {
    LocalSuspicionMetre::activeLocalSusCause activeCause = (LocalSuspicionMetre::activeLocalSusCause)(buffType);
    NCL::CSC8503::ClientSyncLocalActiveSusCausePacket packet(playerNo, activeCause, toApply);
    mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncLocalSusChangePacket(int playerNo, int changedValue) const {
    NCL::CSC8503::ClientSyncLocalSusChangePacket packet(playerNo, changedValue);
    mThisServer->SendGlobalPacket(packet);
}

GameClient* DebugNetworkedGame::GetClient() const{
    return mThisClient;
}

GameServer* DebugNetworkedGame::GetServer() const{
    return mThisServer;
}

NetworkPlayer* DebugNetworkedGame::GetLocalPlayer() const {
    return static_cast<NetworkPlayer*>(mLocalPlayer);
}

void DebugNetworkedGame::UpdateAsServer(float dt){
    mPacketsToSnapshot--;
    if (mPacketsToSnapshot < 0){
        BroadcastSnapshot(false);
        mPacketsToSnapshot = 5;
    }
    else{
        BroadcastSnapshot(true);
    }
}

void DebugNetworkedGame::UpdateAsClient(float dt){
    mThisClient->UpdateClient();

    int peer = GetPlayerPeerID();
    StringPacket newPacket("Client sends it regards :O ");
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)){
        mThisClient->SendPacket(newPacket);
    }
}

void DebugNetworkedGame::BroadcastSnapshot(bool deltaFrame){
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;

    mLevelManager->GetGameWorld()->GetObjectIterators(first, last);

    for (auto i = first; i != last; ++i){
        NetworkObject* o = (*i)->GetNetworkObject();
        if (!o){
            continue;
        }
        //TODO - you'll need some way of determining
        //when a player has sent the server an acknowledgement
        //and store the lastID somewhere. A map between player
        //and an int could work, or it could be part of a 
        //NetworkPlayer struct. 
        int playerState = o->GetLatestNetworkState().stateID;
        GamePacket* newPacket = nullptr;
        if (o->WritePacket(&newPacket, deltaFrame, mServerSideLastFullID)){
            mThisServer->SendGlobalPacket(*newPacket);
            delete newPacket;
        }
    }
}

void DebugNetworkedGame::UpdateMinimumState(){
    //Periodically remove old data from the server
    int minID = INT_MAX;
    int maxID = 0; //we could use this to see if a player is lagging behind?

    for (auto i : mStateIDs){
        minID = std::min(minID, i.second);
        maxID = std::max(maxID, i.second);
    }
    //every client has acknowledged reaching at least state minID
    //so we can get rid of any old states!
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    mLevelManager->GetGameWorld()->GetObjectIterators(first, last);

    for (auto i = first; i != last; ++i){
        NetworkObject* o = (*i)->GetNetworkObject();
        if (!o){
            continue;
        }
        o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
    }
}

int DebugNetworkedGame::GetPlayerPeerID(int peerId){
    if (peerId == -2){
        peerId = mThisClient->GetPeerID();
    }
    for (int i = 0; i < 4; ++i){
        if (mPlayerList[i] == peerId){
            return i;
        }
    }
    return -1;
}

const int DebugNetworkedGame::GetClientLastFullID() const {
    return mClientSideLastFullID;
}

void DebugNetworkedGame::SendStartGameStatusPacket(){
    GameStartStatePacket state(mIsGameStarted);
    mThisServer->SendGlobalPacket(state);
}

void DebugNetworkedGame::SendFinishGameStatusPacket(){
    GameEndStatePacket packet(mIsGameFinished);
    mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::InitWorld(){
    mLevelManager->GetGameWorld()->ClearAndErase();
    mLevelManager->GetPhysics()->Clear();

    //TODO(erendgrmc): Second parameter is redundant remove it from func.
    mLevelManager->LoadLevel(LEVEL_NUM, 0, true);

    SpawnPlayers();
}

void DebugNetworkedGame::HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID){
    //TODO(erendgrmc)
}

void DebugNetworkedGame::SpawnPlayers(){
    
    for (int i = 0; i < 4; i++)		{
        if (mPlayerList[i] != -1) {

            const Vector3& pos = mLevelManager->GetPlayerStartPosition(i);
            auto* netPlayer = AddPlayerObject(pos, i);
            mServerPlayers.emplace(i, netPlayer);
        }
        else
        {
            mServerPlayers.emplace(i, nullptr);
        }
    }
    int playerPeerId = 0;
    if (mThisServer) {
        mLocalPlayer = mServerPlayers[playerPeerId];
    }
    else
    {
        playerPeerId = GetPlayerPeerID();
        mLocalPlayer = mServerPlayers[playerPeerId];
    }
    
    mServerPlayers[playerPeerId]->SetIsLocalPlayer(true);
    mLevelManager->SetTempPlayer((PlayerObject*)mLocalPlayer);
    mLocalPlayer->ToggleIsRendered();
}

NetworkPlayer* DebugNetworkedGame::AddPlayerObject(const Vector3& position, int playerNum){

    //Set Player Obj Name
    char buffer[256]; // Adjust the size according to your needs
    strcpy_s(buffer, sizeof(buffer), _strdup(PLAYER_PREFIX));
    strcat_s(buffer, sizeof(buffer), std::to_string(playerNum).c_str());
    
    auto* netPlayer = new NetworkPlayer(this, playerNum, buffer);
    mLevelManager->CreatePlayerObjectComponents(*netPlayer, position);

    auto* networkComponet = new NetworkObject(*netPlayer, playerNum);
    netPlayer->SetNetworkObject(networkComponet);
    mNetworkObjects.push_back(netPlayer->GetNetworkObject());
    mLevelManager->GetGameWorld()->AddGameObject(netPlayer);
    mLevelManager->AddUpdateableGameObject(*netPlayer);
    Vector4 colour;
    switch (playerNum)
    {
    case 0:
        colour = Vector4(1, 0, 0, 1); // RED
        break;
    case 1:
        colour = Vector4(0, 1, 0, 1); //Green
        break;
    case 2:
        colour = Vector4(0, 0, 1, 1); //Blue
        case 3:
            colour = Vector4(1, 1, 0, 1); //Yellow
        default:
            break;
    }

    netPlayer->GetRenderObject()->SetColour(colour);
    return netPlayer;
}

void DebugNetworkedGame::HandleFullPacket(FullPacket* fullPacket){
    for (int i = 0; i < mNetworkObjects.size(); i++){
        if (mNetworkObjects[i]->GetnetworkID() == fullPacket->objectID){
            mNetworkObjects[i]->ReadPacket(*fullPacket);
        }
    }
    mClientSideLastFullID = fullPacket->fullState.stateID;
}

void DebugNetworkedGame::HandleDeltaPacket(DeltaPacket* deltaPacket){
    for (int i = 0; i < mNetworkObjects.size(); i++){
        if (mNetworkObjects[i]->GetnetworkID() == deltaPacket->objectID){
            mNetworkObjects[i]->ReadPacket(*deltaPacket);
        }
    }
}

void DebugNetworkedGame::HandleClientPlayerInputPacket(ClientPlayerInputPacket* clientPlayerInputPacket, int playerPeerId){
    int playerIndex = GetPlayerPeerID(playerPeerId);
    auto* playerToHandle = mServerPlayers[playerIndex];

    playerToHandle->SetPlayerInput(clientPlayerInputPacket->playerInputs);
    mServerSideLastFullID = clientPlayerInputPacket->lastId;
}

void DebugNetworkedGame::HandleAddPlayerScorePacket(AddPlayerScorePacket* packet){
}

void DebugNetworkedGame::SyncPlayerList(){
    int peerId;
    mPlayerList[0] = 0;
    for (int i = 0; i < 3; ++i){
        if (mThisServer->GetPeer(i, peerId)){
            mPlayerList[i + 1] = peerId;
        }
        else{
            mPlayerList[i + 1] = -1;
        }
    }

    SyncPlayerListPacket packet(mPlayerList);
    mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SetItemsLeftToZero(){
}

void DebugNetworkedGame::HandlePlayerEquippedItemChange(ClientSyncItemSlotPacket* packet) const {
	const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
    auto* inventorySystem = mLevelManager->GetInventoryBuffSystem()->GetPlayerInventoryPtr();
    const PlayerInventory::item equippedItem =  static_cast<PlayerInventory::item>(packet->equippedItem);
	inventorySystem->ChangePlayerItem(packet->playerID, localPlayerID, packet->slotId, equippedItem, packet->usageCount);
}

void DebugNetworkedGame::HandlePlayerBuffChange(ClientSyncBuffPacket* packet) const{
    const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
    auto* buffSystem = mLevelManager->GetInventoryBuffSystem()->GetPlayerBuffsPtr();
    const PlayerBuffs::buff buffToSync = static_cast<PlayerBuffs::buff>(packet->buffID);
    buffSystem->SyncPlayerBuffs(packet->playerID, localPlayerID, buffToSync, packet->toApply);
}

void DebugNetworkedGame::HandleLocalActiveSusCauseChange(ClientSyncLocalActiveSusCausePacket* packet) const{
    const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
    auto* localSusMetre = mLevelManager->GetSuspicionSystem()->GetLocalSuspicionMetre();
    const LocalSuspicionMetre::activeLocalSusCause activeCause = static_cast<LocalSuspicionMetre::activeLocalSusCause>(packet->activeLocalSusCauseID);
    localSusMetre->SyncActiveSusCauses(packet->playerID, localPlayerID, activeCause, packet->toApply);
}

void DebugNetworkedGame::HandleLocalSusChange(ClientSyncLocalSusChangePacket* packet) const {
    const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
    auto* localSusMetre = mLevelManager->GetSuspicionSystem()->GetLocalSuspicionMetre();
    localSusMetre->SyncSusChange(packet->playerID, localPlayerID, packet->changedValue);
} 