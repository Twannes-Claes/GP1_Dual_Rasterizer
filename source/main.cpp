#include "pch.h"
#include "Windows.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"
#include  <conio.h>

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//code to let the color change work
	DWORD consoleMode;

	const HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (GetConsoleMode(outputHandle, &consoleMode))
	{
		SetConsoleMode(outputHandle, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	constexpr uint32_t width = 640;
	constexpr uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"Dual Rasterizer - **Twannes Claes(2DAE15)",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:


				//logic for switching bools and enums

				if (e.key.keysym.scancode == SDL_SCANCODE_RETURN)
				{
					std::cout << "\x1B[2J\x1B[H"; //jumps to next part of unwritten console
					pRenderer->PrintInstructions();
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_LCTRL) pRenderer->ToggleCameraLock();

				if (e.key.keysym.scancode == SDL_SCANCODE_F1) pRenderer->ToggleBackgroundState();

				if (e.key.keysym.scancode == SDL_SCANCODE_F2) pRenderer->ToggleRotation();

				if (e.key.keysym.scancode == SDL_SCANCODE_F3) pRenderer->ToggleFireMesh();

				if (e.key.keysym.scancode == SDL_SCANCODE_F4) pRenderer->ToggleSampleState();

				if (e.key.keysym.scancode == SDL_SCANCODE_F5) pRenderer->ToggleSoftwareState();

				if (e.key.keysym.scancode == SDL_SCANCODE_F6) pRenderer->ToggleNormal();

				if (e.key.keysym.scancode == SDL_SCANCODE_F7) pRenderer->ToggleDepth();

				if (e.key.keysym.scancode == SDL_SCANCODE_F8) pRenderer->ToggleBounding();

				if (e.key.keysym.scancode == SDL_SCANCODE_F9) pRenderer->ToggleCullMode();

				if (e.key.keysym.scancode == SDL_SCANCODE_F10) pRenderer->ToggleUniform();

				if (e.key.keysym.scancode == SDL_SCANCODE_F11) pRenderer->TogglePrintingFPS();

				break;

			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		
		pTimer->Update();

		//if able to print -> print
		if (pRenderer->CanPrintFPS())
		{
			printTimer += pTimer->GetElapsed();

			if (printTimer >= 1.f)
			{
				printTimer = 0.f;
				std::cout << "\033[37m"; //print fps in white
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
		
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}