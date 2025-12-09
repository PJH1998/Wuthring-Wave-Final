#pragma once
#include "ClientPch.h"

typedef struct tagLeviatanGrabQTE : public CEvent //강제 속박 QTE 결과 이벤트
{
	_bool isSuccess;
	tagLeviatanGrabQTE(_bool _isSuccess) : isSuccess{ _isSuccess } {};
}LEVI_GRAB; 

typedef struct tagLeviatanExecution : public CEvent // 처형 결과 이벤트
{
	_bool isSuccess;
	tagLeviatanExecution(_bool _isSuccess) : isSuccess{ _isSuccess } {};
}LEVI_EXECUTE;