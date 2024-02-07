#include "Shader.h"

LPVOID FShader::GetBufferPointer()
{
	return ShaderCode->GetBufferPointer();
}

SIZE_T FShader::GetBufferSize()
{
	return ShaderCode->GetBufferSize();
}

void FShader::BuildShaders(const wstring& InFileName, const string& InEntryFuncName, const string& InShaderVersion)
{
	ComPtr<ID3DBlob> ErrorShaderMsg;
	HRESULT R = D3DCompileFromFile(
		InFileName.c_str(),
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		InEntryFuncName.c_str(),
		InShaderVersion.c_str(),
#if _DEBUG
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION
#else
		0
#endif
		,0,
		&ShaderCode,
		&ErrorShaderMsg);

	if (ErrorShaderMsg)
	{
		ENGINE_LOG_ERROR("%s", (char*)ErrorShaderMsg->GetBufferPointer());
	}

	ANALYSIS_HRESULT(R);
}