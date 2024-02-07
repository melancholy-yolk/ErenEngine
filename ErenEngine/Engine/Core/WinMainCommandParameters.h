#pragma once

#if defined(_WIN32)
#include "../EngineMinimal.h"
class FWinMainCommandParameters
{
public:
	FWinMainCommandParameters(HINSTANCE InHInstance, HINSTANCE InPrevInstance, PSTR InCmdLine, int InShowCmd);

	HINSTANCE HInstance;
	HINSTANCE PrevInstance;
	PSTR CmdLine;
	int ShowCmd;
};
#elif defined(__linux__)

#endif
