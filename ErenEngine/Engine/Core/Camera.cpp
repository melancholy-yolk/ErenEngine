#include "Camera.h"
#include "../Component/Input/InputType.h"
#include "../Component/TransformComponent.h"
#include "../Component/InputComponent.h"

CCamera::CCamera()
{
	TransformComponent = CreateObject<CTransformComponent>(new CTransformComponent());
	InputComponent = CreateObject<CInputComponent>(new CInputComponent());
}

void CCamera::BeginInit()
{
	InputComponent->CaptureKeyboardInfoDelegate.Bind(this, &CCamera::ExecuteKeyboard);
}

void CCamera::Tick(float DeltaTime)
{

}

void CCamera::ExecuteKeyboard(const FInputKey& InputKey)
{

}
