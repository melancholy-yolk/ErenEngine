#include "EngineMinimal.h"
#include "EngineFactory.h"
#include "Debug/Log/SimpleLog.h"

int Init(CEngine* InEngine, HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(_WIN32)
	FWinMainCommandParameters WinMainCommandParameters(hInstance, prevInstance, cmdLine, showCmd);
#endif

	int ReturnCode = InEngine->PreInit(
#if defined(_WIN32)
		WinMainCommandParameters
#endif
	);

	if (ReturnCode != 0)
	{
		ENGINE_LOG_ERROR("Engine PreInit Error: %i", ReturnCode);
		return ReturnCode;
	}

	ReturnCode = InEngine->Init(
#if defined(_WIN32)
		WinMainCommandParameters
#endif
	);
	if (ReturnCode != 0)
	{
		ENGINE_LOG_ERROR("Engine Init Error: %i", ReturnCode);
		return ReturnCode;
	}

	ReturnCode = InEngine->PostInit();
	if (ReturnCode != 0)
	{
		ENGINE_LOG_ERROR("Engine PostInit Error: %i", ReturnCode);
		return ReturnCode;
	}

	return ReturnCode;
}

void Tick(CEngine* InEngine)
{
	float DeltaTime = 0.03f;
	InEngine->Tick(DeltaTime);

	//Sleep(30);
}

int Exit(CEngine* InEngine)
{
	int ReturnCode = InEngine->PreExit();
	if (ReturnCode != 0)
	{
		ENGINE_LOG_ERROR("Engine PreExit Error: %i", ReturnCode);
		return ReturnCode;
	}

	ReturnCode = InEngine->Exit();
	if (ReturnCode != 0)
	{
		ENGINE_LOG_ERROR("Engine Exit Error: %i", ReturnCode);
		return ReturnCode;
	}

	ReturnCode = InEngine->PostExit();
	if (ReturnCode != 0)
	{
		ENGINE_LOG_ERROR("Engine PostExit Error: %i", ReturnCode);
		return ReturnCode;
	}

	return ReturnCode;
}

CEngine* Engine = NULL;

// 自己的实例
// 上次的实例
// 命令行参数
// 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	int ReturnCode = 0;

	Engine = FEngineFactory::CreateEngine();
	if (Engine)
	{
		// 初始化
		Init(Engine, hInstance, prevInstance, cmdLine, showCmd);

		MSG EngineMsg = { 0 };

		while (EngineMsg.message != WM_QUIT)
		{
			if (PeekMessage(&EngineMsg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&EngineMsg);
				DispatchMessage(&EngineMsg);
			}
			else
			{
				Tick(Engine);
			}
		}

		ReturnCode = Exit(Engine);
	}
	else
	{
		ReturnCode = 1;
	}
	
	ENGINE_LOG("Engine Exit: %i", ReturnCode);
	return ReturnCode;
}