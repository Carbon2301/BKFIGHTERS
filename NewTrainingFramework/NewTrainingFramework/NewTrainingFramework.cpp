// NewTrainingFramework.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Vertex.h"
#include "Shaders.h"
#include "Globals.h"
#include "Model.h"
#include <conio.h>
#include "../Utilities/utilities.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Model girlModel;
Shaders myShaders;

// Camera variables
Vector3 cameraPos(0.0f, 0.5f, 3.0f);
Vector3 cameraTarget(0.0f, 0.0f, 0.0f);
Vector3 cameraUp(0.0f, 1.0f, 0.0f);
float modelRotationY = 0.0f;

int Init(ESContext* esContext)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// Enable depth testing for 3D model
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Try loading different models
	const char* modelFiles[] = {
		"../Resources/Models/Woman1.nfg",  
		"../Resources/Models/Woman2.nfg"
	};
	
	const char* textureFiles[] = {
		"../Resources/Textures/Woman1.tga",
		"../Resources/Textures/Woman2.tga"
	};
	
	// Try loading first available model
	bool modelLoaded = false;
	int numModels = sizeof(modelFiles) / sizeof(modelFiles[0]);
	for (int i = 0; i < numModels; i++) {
		if (girlModel.LoadFromNFG(modelFiles[i])) {
			std::cout << "✅ Loaded model: " << modelFiles[i] << std::endl;
			
			// Try loading corresponding texture
			if (girlModel.LoadTexture(textureFiles[i])) {
				std::cout << "✅ Loaded texture: " << textureFiles[i] << std::endl;
			} else {
				std::cout << "⚠️ No texture found, using vertex colors" << std::endl;
			}
			
			modelLoaded = true;
			break;
		}
	}
	
	if (!modelLoaded) {
		std::cout << "❌ No models found!" << std::endl;
		return -1;
	}
	
	// Create buffers for the model
	girlModel.CreateBuffers();

	//creation of shaders and program 
	return myShaders.Init("../Resources/Shaders/TriangleShaderVS.vs", "../Resources/Shaders/TriangleShaderFS.fs");
}

void Draw(ESContext* esContext)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(myShaders.program);

	// === MVP Matrix Setup ===
	
	// 1. Model Matrix (World transformation)
	Matrix modelMatrix;
	modelMatrix.SetIdentity();
	
	// Center model: Woman model Y từ 0.3->1.8, center ở 1.05
	Matrix translation;
	translation.SetTranslation(0.0f, 0.0f, 0.0f);
	
	// Scale model down để phù hợp với scene
	Matrix scale;
	scale.SetScale(0.5f, 0.5f, 0.5f);
	
	// Rotate model for animation
	Matrix rotation;
	rotation.SetRotationY(modelRotationY);
	
	// Combine: Model = T × R × S (right to left multiplication)
	modelMatrix = translation * rotation * scale;
	
	// 2. View Matrix (Camera transformation)
	Matrix viewMatrix;
	viewMatrix.SetLookAt(cameraPos, cameraTarget, cameraUp);
	
	// 3. Projection Matrix
	Matrix projMatrix;
	float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
	projMatrix.SetPerspective(45.0f * M_PI / 180.0f, aspect, 0.1f, 100.0f);
	
	// 4. MVP = Model × View × Projection (row-major order)
	Matrix mvpMatrix = modelMatrix * viewMatrix * projMatrix;
	
	// Send MVP matrix to shader
	GLint mvpLocation = glGetUniformLocation(myShaders.program, "u_mvpMatrix");
	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvpMatrix.m[0][0]);

	// Set texture uniform
	GLint textureLocation = glGetUniformLocation(myShaders.program, "u_texture");
	glUniform1i(textureLocation, 0);  // Bind to texture unit 0

	// Draw the girl model
	girlModel.Draw();

	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

void Update ( ESContext *esContext, float deltaTime )
{
	// Auto-rotate model
	modelRotationY += deltaTime * 0.5f; // 0.5 radians per second
	if (modelRotationY > 2.0f * M_PI) {
		modelRotationY -= 2.0f * M_PI;
	}
}

void Key ( ESContext *esContext, unsigned char key, bool bIsPressed)
{
	if (bIsPressed) {
		float moveSpeed = 0.1f;
		switch(key) {
			// === Camera Controls ===
			case 'W':
			case 'w':
				// Move camera forward
				cameraPos.z -= moveSpeed;
				std::cout << "Camera moved forward" << std::endl;
				break;
			case 'S':
			case 's':
				// Move camera backward  
				cameraPos.z += moveSpeed;
				std::cout << "Camera moved backward" << std::endl;
				break;
			case 'A':
			case 'a':
				// Move camera left
				cameraPos.x -= moveSpeed;
				std::cout << "Camera moved left" << std::endl;
				break;
			case 'D':
			case 'd':
				// Move camera right
				cameraPos.x += moveSpeed;
				std::cout << "Camera moved right" << std::endl;
				break;
			case 'Q':
			case 'q':
				// Move camera up
				cameraPos.y += moveSpeed;
				std::cout << "Camera moved up" << std::endl;
				break;
			case 'E':
			case 'e':
				// Move camera down
				cameraPos.y -= moveSpeed;
				std::cout << "Camera moved down" << std::endl;
				break;
				
			// === Rendering Modes ===
			case '1': 
				std::cout << "Switched to pure texture mode" << std::endl;
				// User can uncomment option 1 in fragment shader
				break;
			case '2':
				std::cout << "Switched to rainbow normal mode (current)" << std::endl; 
				// Current mode
				break;
			case '3':
				std::cout << "Switched to pure vertex color mode" << std::endl;
				// User can uncomment option 3 in fragment shader
				break;
				
			// === Info ===
			case 'R':
			case 'r':
				std::cout << "=== Camera Info ===" << std::endl;
				std::cout << "Camera Position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
				std::cout << "Model Rotation: " << modelRotationY * 180.0f / M_PI << " degrees" << std::endl;
				std::cout << "Controls: WASD=move, QE=up/down, 1-3=render modes" << std::endl;
				break;
		}
	}
}

void CleanUp()
{
	girlModel.Cleanup();
}

int _tmain(int argc, _TCHAR* argv[])
{
	ESContext esContext;

    esInitContext ( &esContext );

	esCreateWindow ( &esContext, "Hello Triangle", Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB | ES_WINDOW_DEPTH);

	if ( Init ( &esContext ) != 0 )
		return 0;

	esRegisterDrawFunc ( &esContext, Draw );
	esRegisterUpdateFunc ( &esContext, Update );
	esRegisterKeyFunc ( &esContext, Key);

	esMainLoop ( &esContext );
	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
	//releasing OpenGL resources
	CleanUp();

	//identifying memory leaks
	MemoryDump();
	printf("Press any key...\n");
	_getch();

	return 0;
}

