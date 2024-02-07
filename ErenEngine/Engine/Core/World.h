#pragma once

#include "CoreObject/CoreMinimalObject.h"

class CCamera;

class CWorld : public CCoreMinimalObject
{
public:
	CWorld();
	virtual ~CWorld();

	CVARIABLE()
	CCamera* Camera;
};