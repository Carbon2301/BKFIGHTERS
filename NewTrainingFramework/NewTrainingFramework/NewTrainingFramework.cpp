// NewTrainingFramework.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Vertex.h"
#include "Shaders.h"
#include "Globals.h"
#include "Model.h"
#include <conio.h>
#include "../Utilities/utilities.h" // if you use STL, please include this line AFTER all other include

Model girlModel;
Shaders myShaders;

int Init(ESContext* esContext)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// Enable depth testing for 3D model
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Try loading different models
	const char* modelFiles[] = {
		"../Resources/Models/Girl.nfg",     // Your custom model
		"../Resources/Models/Woman1.nfg",   // Original model  
		"../Resources/Models/Woman2.nfg"    // Alternative model
	};
	
	const char* textureFiles[] = {
		"../Resources/Textures/Girl.tga",
		"../Resources/Textures/Woman1.tga",
		"../Resources/Textures/Woman2.tga"
	};
	
	// Try loading first available model
	bool modelLoaded = false;
	for (int i = 0; i < 3; i++) {
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

	// Set texture uniform
	GLint textureLocation = glGetUniformLocation(myShaders.program, "u_texture");
	glUniform1i(textureLocation, 0);  // Bind to texture unit 0

	// Draw the girl model
	girlModel.Draw();

	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

void Update ( ESContext *esContext, float deltaTime )
{

}

void Key ( ESContext *esContext, unsigned char key, bool bIsPressed)
{
	if (bIsPressed) {
		switch(key) {
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
			case 'r':
			case 'R':
				std::cout << "Model rendering info logged" << std::endl;
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

