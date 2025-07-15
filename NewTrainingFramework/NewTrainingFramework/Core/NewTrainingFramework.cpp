// NewTrainingFramework.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../GameObject/Vertex.h"
#include "../GameObject/Shaders.h"
#include "Globals.h"
#include "../GameObject/Model.h"
#include "../GameManager/ResourceManager.h"
#include "../GameManager/SceneManager.h"
#include "../GameManager/GameStateMachine.h"
#include "../GameObject/Object.h"
#include "../GameObject/Camera.h"
#include <conio.h>
#include "../../Utilities/utilities.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Engine systems
ResourceManager* g_resourceManager = nullptr;
SceneManager* g_sceneManager = nullptr;
GameStateMachine* g_gameStateMachine = nullptr;

// Game state flag
bool g_useGameStateMachine = true;

int Init(ESContext* esContext)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// Enable depth testing (still needed for proper rendering)
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	std::cout << "\n=== 2D Game Engine with State Machine ===" << std::endl;
	std::cout << "Initializing New Training Framework Engine..." << std::endl;
	
	// Initialize Resource Manager
	g_resourceManager = ResourceManager::GetInstance();
	if (!g_resourceManager->LoadFromFile("RM.txt")) {
		std::cout << "Failed to load resources!" << std::endl;
		return -1;
	}
	
	// Initialize Scene Manager (for compatibility)
	g_sceneManager = SceneManager::GetInstance();
	if (!g_sceneManager->LoadFromFile("SM.txt")) {
		std::cout << "Failed to load scene!" << std::endl;
		return -1;
	}
	
	// Initialize Game State Machine
	g_gameStateMachine = GameStateMachine::GetInstance();
	
	std::cout << "Engine initialized successfully!" << std::endl;
	std::cout << "\n=== 2D Game State Machine ===" << std::endl;
	std::cout << "Starting with Loading Screen..." << std::endl;
	std::cout << "State Flow: Loading -> Menu -> Play" << std::endl;
	std::cout << "\nGame Controls will be shown in each state." << std::endl;
	std::cout << "==========================================\n" << std::endl;
	
	// Start with intro/loading state
	g_gameStateMachine->ChangeState(StateType::INTRO);
	
	return 0;
}

void Draw(ESContext* esContext)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use Game State Machine for rendering
	if (g_useGameStateMachine && g_gameStateMachine) {
		g_gameStateMachine->Draw();
	}
	// Fallback to SceneManager (for debugging/compatibility)
	else if (g_sceneManager) {
		g_sceneManager->Draw();
	}

	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);	
}

void Update(ESContext *esContext, float deltaTime)
{
	// Use Game State Machine for updates
	if (g_useGameStateMachine && g_gameStateMachine) {
		g_gameStateMachine->Update(deltaTime);
	}
	// Fallback to SceneManager (for debugging/compatibility)
	else if (g_sceneManager) {
		g_sceneManager->Update(deltaTime);
	}
}

void Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	// Use Game State Machine for input handling
	if (g_useGameStateMachine && g_gameStateMachine) {
		g_gameStateMachine->HandleKeyEvent(key, bIsPressed);
	}
	// Fallback to SceneManager (for debugging/compatibility)
	else if (bIsPressed && g_sceneManager) {
		g_sceneManager->HandleInput(key, bIsPressed);
	}
	
	// Global engine controls (work in any mode)
	if (bIsPressed) {
		switch(key) {
			case '`': // Backtick - toggle between Game State Machine and SceneManager
				g_useGameStateMachine = !g_useGameStateMachine;
				std::cout << "Switched to: " << (g_useGameStateMachine ? "Game State Machine" : "Scene Manager (3D Mode)") << std::endl;
				break;
		}
	}
}

void CleanUp()
{
	// Cleanup engine systems
	if (g_gameStateMachine) {
		GameStateMachine::DestroyInstance();
		g_gameStateMachine = nullptr;
	}
	
	if (g_sceneManager) {
		SceneManager::DestroyInstance();
		g_sceneManager = nullptr;
	}
	
	if (g_resourceManager) {
		ResourceManager::DestroyInstance();
		g_resourceManager = nullptr;
	}
	
	std::cout << "🧹 2D Game Engine cleanup completed" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	ESContext esContext;

	esInitContext ( &esContext );

	esCreateWindow ( &esContext, "New Training Framework - Engine Architecture", Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB | ES_WINDOW_DEPTH);

	if ( Init ( &esContext ) != 0 )
		return 0;

	esRegisterDrawFunc ( &esContext, Draw );
	esRegisterUpdateFunc ( &esContext, Update );
	esRegisterKeyFunc ( &esContext, Key);

	esMainLoop ( &esContext );

	//releasing OpenGL resources
	CleanUp();

	//identifying memory leaks
	MemoryManager::GetInstance()->SanityCheck();

	printf("Press any key...\n");
	_getch();

	return 0;
}

