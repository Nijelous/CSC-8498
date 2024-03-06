﻿#include "SceneStates.h"
#include "Scene.h"

#include "DebugNetworkedGame.h"
#include "Window.h"
#include "SceneManager.h"
#include "MainMenuScene.h"

using namespace NCL::CSC8503;

#ifdef USEGL

void MainMenuSceneState::OnAwake() {	
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::MainMenu);
}

PushdownState::PushdownResult MainMenuSceneState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1)) {
		*newState = new SingleplayerState();
		return PushdownResult::Push;
	}
	
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2)) {
		*newState = new ServerState();
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3)) {
		*newState = new ClientState();
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		SceneManager::GetSceneManager()->SetIsForceQuit(true);
		return PushdownResult::Pop;
	}
	return PushdownResult::NoChange;
}

PushdownState::PushdownResult SingleplayerState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE) && LevelManager::GetLevelManager()->GetGameState() == MenuState) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void SingleplayerState::OnAwake() {
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Singleplayer);
	auto* gameScene = (GameSceneManager*)(SceneManager::GetSceneManager()->GetCurrentScene());
}

PushdownState::PushdownResult ServerState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void ServerState::OnAwake(){
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Multiplayer);
	SceneManager::GetSceneManager()->SetIsServer(true);
	auto* server = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	server->StartAsServer();
}

PushdownState::PushdownResult ClientState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void ClientState::OnAwake() {
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Multiplayer);
	SceneManager::GetSceneManager()->SetIsServer(false);
	auto* client = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	//client->StartAsClient(10,58,221,142);
	//Localhost IP
	client->StartAsClient(127, 0, 0, 1);
}
#endif