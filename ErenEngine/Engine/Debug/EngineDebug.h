#pragma once

#define ANALYSIS_HRESULT(InValue) \
{\
	HRESULT HandleResult = InValue;\
	if (FAILED(HandleResult))\
	{\
		ENGINE_LOG_ERROR("Error = %i", (int)HandleResult);\
		assert(0);\
	}\
	else if (SUCCEEDED(HandleResult))\
	{\
		ENGINE_LOG_SUCCESS("Success = %i", (int)HandleResult);\
	}\
}