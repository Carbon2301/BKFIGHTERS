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

int Init(ESContext* esContext)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// 2D rendering - depth testing not needed
	// glEnable(GL_DEPTH_TEST);  // Disabled for 2D-only engine
	// glDepthFunc(GL_LEQUAL);   // Disabled for 2D-only engine

	std::cout << "\n=== 2D Game Engine with State Machine ===" << std::endl;
	std::cout << "Initializing New Training Framework Engine..." << std::endl;
	
	// Initialize Resource Manager
	g_resourceManager = ResourceManager::GetInstance();
	if (!g_resourceManager->LoadFromFile("RM.txt")) {
		std::cout << "Failed to load resources!" << std::endl;
		return -1;
	}
	
	// Initialize Scene Manager (scenes will be loaded per-state)
	g_sceneManager = SceneManager::GetInstance();
	std::cout << "SceneManager initialized (scenes loaded per-state)" << std::endl;
	
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
	glClear(GL_COLOR_BUFFER_BIT);  // 2D-only: No depth buffer clearing needed

	// Use Game State Machine for rendering (2D-only)
	if (g_gameStateMachine) {
		g_gameStateMachine->Draw();
	}

	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);	
}

void Update(ESContext *esContext, float deltaTime)
{
	// Use Game State Machine for updates (2D-only)
	if (g_gameStateMachine) {
		g_gameStateMachine->Update(deltaTime);
	}
}

void Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	// Use Game State Machine for input handling (2D-only)
	if (g_gameStateMachine) {
		g_gameStateMachine->HandleKeyEvent(key, bIsPressed);
	}
}

void MouseClick(ESContext *esContext, int x, int y, bool bIsPressed)
{
	// Use Game State Machine for mouse input handling (2D-only)
	if (g_gameStateMachine) {
		g_gameStateMachine->HandleMouseEvent(x, y, bIsPressed);
	}
}

void OnMouseMove(ESContext *esContext, int x, int y)
{
	// Use Game State Machine for mouse movement handling (2D-only)
	if (g_gameStateMachine) {
		g_gameStateMachine->HandleMouseMove(x, y);
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

	esCreateWindow ( &esContext, "New Training Framework - 2D Engine", Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB);  // 2D-only: No depth buffer needed

	if ( Init ( &esContext ) != 0 )
		return 0;

	esRegisterDrawFunc ( &esContext, Draw );
	esRegisterUpdateFunc ( &esContext, Update );
	esRegisterKeyFunc ( &esContext, Key);
	esRegisterMouseFunc ( &esContext, MouseClick );
	esRegisterMouseMoveFunc ( &esContext, OnMouseMove );

	esMainLoop ( &esContext );

	//releasing OpenGL resources
	CleanUp();

	//identifying memory leaks
	MemoryManager::GetInstance()->SanityCheck();

	printf("Press any key...\n");
	_getch();

	return 0;
}

