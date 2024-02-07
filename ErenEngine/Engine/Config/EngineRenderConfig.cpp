#include "EngineRenderConfig.h"

FEngineRenderConfig* FEngineRenderConfig::EngineRenderConfig = nullptr;

FEngineRenderConfig::FEngineRenderConfig()
	: ScreenWidth(1280),
	ScreenHeight(720),
	RefreshRate(60),
	SwapChainBufferCount(2)
{

}

FEngineRenderConfig* FEngineRenderConfig::GetEngineRenderConfig()
{
	if (!EngineRenderConfig)
	{
		EngineRenderConfig = new FEngineRenderConfig();
	}
	return EngineRenderConfig;
}

void FEngineRenderConfig::Destroy()
{
	if (EngineRenderConfig)
	{
		delete EngineRenderConfig;
		EngineRenderConfig = nullptr;
	}
}
