// NewTrainingFramework.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
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
#include "../GameManager/SoundManager.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ResourceManager* g_resourceManager = nullptr;
SceneManager* g_sceneManager = nullptr;
GameStateMachine* g_gameStateMachine = nullptr;

int Init(ESContext* esContext)
{
    glClearColor(0.380f, 0.643f, 0.871f, 1.0f);
	g_resourceManager = ResourceManager::GetInstance();
	if (!g_resourceManager->LoadFromFile("../Resources/RM.txt")) {
		return -1;
	}
	
	SoundManager::Instance().LoadMusicFromFile("../Resources/RM.txt");
	
	g_sceneManager = SceneManager::GetInstance();
	
	g_gameStateMachine = GameStateMachine::GetInstance();
	
	g_gameStateMachine->ChangeState(StateType::INTRO);
	
	return 0;
}

void Draw(ESContext* esContext)
{
    RECT clientRect;
    GetClientRect((HWND)esContext->hWnd, &clientRect);
    int clientW = clientRect.right - clientRect.left;
    int clientH = clientRect.bottom - clientRect.top;
    const float logicalW = static_cast<float>(Globals::screenWidth);
    const float logicalH = static_cast<float>(Globals::screenHeight);
    float scale = 1.0f;
    int vpX = 0, vpY = 0, vpW = clientW, vpH = clientH;
    if (clientW > 0 && clientH > 0) {
        const float scaleX = static_cast<float>(clientW) / logicalW;
        const float scaleY = static_cast<float>(clientH) / logicalH;
        scale = (scaleX < scaleY) ? scaleX : scaleY;
        vpW = static_cast<int>(logicalW * scale + 0.5f);
        vpH = static_cast<int>(logicalH * scale + 0.5f);
        vpX = (clientW - vpW) / 2;
        vpY = (clientH - vpH) / 2;
    }

    glViewport(vpX, vpY, vpW, vpH);
    glClear(GL_COLOR_BUFFER_BIT);

	if (g_gameStateMachine) {
		g_gameStateMachine->Draw();
	}

	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);	
}

void Update(ESContext *esContext, float deltaTime)
{
	if (g_gameStateMachine) {
		g_gameStateMachine->Update(deltaTime);
	}
}

void Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	if (g_gameStateMachine) {
		g_gameStateMachine->HandleKeyEvent(key, bIsPressed);
	}
}

void MouseClick(ESContext *esContext, int x, int y, bool bIsPressed)
{
    if (g_gameStateMachine) {
        // Map from window pixels to logical coordinates (1280x720)
        RECT clientRect;
        GetClientRect((HWND)esContext->hWnd, &clientRect);
        int clientW = clientRect.right - clientRect.left;
        int clientH = clientRect.bottom - clientRect.top;
        const float logicalW = static_cast<float>(Globals::screenWidth);
        const float logicalH = static_cast<float>(Globals::screenHeight);
        int vpX = 0, vpY = 0, vpW = clientW, vpH = clientH;
        float scale = 1.0f;
        if (clientW > 0 && clientH > 0) {
            const float scaleX = static_cast<float>(clientW) / logicalW;
            const float scaleY = static_cast<float>(clientH) / logicalH;
            scale = (scaleX < scaleY) ? scaleX : scaleY;
            vpW = static_cast<int>(logicalW * scale + 0.5f);
            vpH = static_cast<int>(logicalH * scale + 0.5f);
            vpX = (clientW - vpW) / 2;
            vpY = (clientH - vpH) / 2;
        }
        if (x < vpX || y < vpY || x >= vpX + vpW || y >= vpY + vpH) {
            return;
        }
        int lx = static_cast<int>((x - vpX) / scale);
        int ly = static_cast<int>((y - vpY) / scale);
        g_gameStateMachine->HandleMouseEvent(lx, ly, bIsPressed);
    }
}

void OnMouseMove(ESContext *esContext, int x, int y)
{
    if (g_gameStateMachine) {
        RECT clientRect;
        GetClientRect((HWND)esContext->hWnd, &clientRect);
        int clientW = clientRect.right - clientRect.left;
        int clientH = clientRect.bottom - clientRect.top;
        const float logicalW = static_cast<float>(Globals::screenWidth);
        const float logicalH = static_cast<float>(Globals::screenHeight);
        int vpX = 0, vpY = 0, vpW = clientW, vpH = clientH;
        float scale = 1.0f;
        if (clientW > 0 && clientH > 0) {
            const float scaleX = static_cast<float>(clientW) / logicalW;
            const float scaleY = static_cast<float>(clientH) / logicalH;
            scale = (scaleX < scaleY) ? scaleX : scaleY;
            vpW = static_cast<int>(logicalW * scale + 0.5f);
            vpH = static_cast<int>(logicalH * scale + 0.5f);
            vpX = (clientW - vpW) / 2;
            vpY = (clientH - vpH) / 2;
        }
        if (x < vpX || y < vpY || x >= vpX + vpW || y >= vpY + vpH) {
            return;
        }
        int lx = static_cast<int>((x - vpX) / scale);
        int ly = static_cast<int>((y - vpY) / scale);
        g_gameStateMachine->HandleMouseMove(lx, ly);
    }
}

void CleanUp()
{
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
}

int _tmain(int argc, _TCHAR* argv[])
{
	ESContext esContext;

	esInitContext ( &esContext );

    const char* baseTitle = "New Training Framework - 2D Engine";
    const char* title = Globals::fullscreenScale ? "New Training Framework - 2D Engine [FS]" : baseTitle;
    esCreateWindow ( &esContext, title, Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB);

	if ( Init ( &esContext ) != 0 )
		return 0;

	esRegisterDrawFunc ( &esContext, Draw );
	esRegisterUpdateFunc ( &esContext, Update );
	esRegisterKeyFunc ( &esContext, Key);
	esRegisterMouseFunc ( &esContext, MouseClick );
	esRegisterMouseMoveFunc ( &esContext, OnMouseMove );

	esMainLoop ( &esContext );

	CleanUp();

	MemoryManager::GetInstance()->SanityCheck();

	printf("Press any key...\n");
	_getch();

	return 0;
}

