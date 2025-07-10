// NewTrainingFramework.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Vertex.h"
#include "Shaders.h"
#include "Globals.h"
#include "Model.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "Object.h"
#include "Camera.h"
#include <conio.h>
#include "../Utilities/utilities.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Engine systems
ResourceManager* g_resourceManager = nullptr;
SceneManager* g_sceneManager = nullptr;

// Auto-rotation for demonstration
float g_autoRotationSpeed = 0.5f;
bool g_enableAutoRotation = true;

int Init(ESContext* esContext)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// Enable depth testing for 3D models
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	std::cout << "\nInitializing New Training Framework Engine..." << std::endl;
	
	// Initialize Resource Manager
	g_resourceManager = ResourceManager::GetInstance();
	if (!g_resourceManager->LoadFromFile("RM.txt")) {
		std::cout << "Failed to load resources!" << std::endl;
		return -1;
	}
	
	// Initialize Scene Manager  
	g_sceneManager = SceneManager::GetInstance();
	if (!g_sceneManager->LoadFromFile("SM.txt")) {
		std::cout << "Failed to load scene!" << std::endl;
		return -1;
	}
	
	std::cout << "Engine initialized successfully!" << std::endl;
	std::cout << "\n=== Controls ===" << std::endl;
	std::cout << "WASD - Move camera" << std::endl;
	std::cout << "QE - Camera up/down" << std::endl;
	std::cout << "IJKL - Look up/left/down/right" << std::endl;
	std::cout << "UO - Roll left/right" << std::endl;
	std::cout << "ZX - Zoom in/out" << std::endl;
	std::cout << "C - Reset zoom" << std::endl;
	std::cout << "R - Show scene info" << std::endl;
	std::cout << "T - Toggle auto-rotation" << std::endl;
	std::cout << "1-3 - Rendering modes (edit fragment shader)" << std::endl;
	std::cout << "===============\n" << std::endl;
	
	return 0;
}

void Draw(ESContext* esContext)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw scene using SceneManager
	if (g_sceneManager) {
		g_sceneManager->Draw();
	}

	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

void Update(ESContext *esContext, float deltaTime)
{
	// Update scene
	if (g_sceneManager) {
		g_sceneManager->Update(deltaTime);
	}
	
	// Auto-rotation demonstration
	if (g_enableAutoRotation && g_sceneManager) {
		// Rotate all objects
		const auto& objects = g_sceneManager->GetObjects();
		for (auto& obj : objects) {
			const Vector3& currentRot = obj->GetRotation();
			Vector3 newRotation(currentRot.x, currentRot.y, currentRot.z);
			newRotation.y += deltaTime * g_autoRotationSpeed;
			if (newRotation.y > 2.0f * (float)M_PI) {
				newRotation.y -= 2.0f * (float)M_PI;
			}
			obj->SetRotation(newRotation);
		}
	}
}

void Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	if (bIsPressed) {
		// Handle scene manager input (camera controls)
		if (g_sceneManager) {
			g_sceneManager->HandleInput(key, bIsPressed);
		}
		
		// Additional engine controls
		switch(key) {
			case 'T':
			case 't':
				g_enableAutoRotation = !g_enableAutoRotation;
				std::cout << "Auto-rotation: " << (g_enableAutoRotation ? "ON" : "OFF") << std::endl;
				break;
				
			// === Rendering Modes Info ===
			case '1': 
				std::cout << "To switch to pure texture mode, edit fragment shader option 1" << std::endl;
				break;
			case '2':
				std::cout << "Current: rainbow normal mode (default)" << std::endl; 
				break;
			case '3':
				std::cout << "To switch to pure vertex color mode, edit fragment shader option 3" << std::endl;
				break;
		}
	}
}

void CleanUp()
{
	// Cleanup engine systems
	if (g_sceneManager) {
		SceneManager::DestroyInstance();
		g_sceneManager = nullptr;
	}
	
	if (g_resourceManager) {
		ResourceManager::DestroyInstance();
		g_resourceManager = nullptr;
	}
	
	std::cout << "Engine cleanup completed" << std::endl;
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

