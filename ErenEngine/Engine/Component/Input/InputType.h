#pragma once

#include "../../EngineMinimal.h"

enum FPressState
{
	PRESS,
	RELEASE
};

struct FInputKey
{
	string KeyName;
	FPressState PressState;
};