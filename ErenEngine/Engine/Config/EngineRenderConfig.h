#pragma once

struct FEngineRenderConfig
{
	FEngineRenderConfig();

	int ScreenWidth;
	int ScreenHeight;
	int RefreshRate;
	int SwapChainBufferCount;

	static FEngineRenderConfig* GetEngineRenderConfig();
	static void Destroy();
private:
	static FEngineRenderConfig* EngineRenderConfig;
};

class FAutoConsoleVariable
{

};

class FConfigCache
{

};