#pragma once

#include "Viewport.h"
#include "CoreObject/CoreMinimalObject.h"

class CTransformComponent;
class CInputComponent;
struct FInputKey;

class CCamera : public CCoreMinimalObject, public FViewport
{
	CVARIABLE()
	CTransformComponent* TransformComponent;

	CVARIABLE()
	CInputComponent* InputComponent;
public:
	CCamera();

	virtual void BeginInit();
	virtual void Tick(float DeltaTime);

	virtual void ExecuteKeyboard(const FInputKey& InputKey);

public:
	FORCEINLINE CTransformComponent* GetTransformComponent() { return TransformComponent; }
	FORCEINLINE CInputComponent* GetInputComponent() { return InputComponent; }
};