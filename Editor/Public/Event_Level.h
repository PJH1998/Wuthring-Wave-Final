#pragma once
#include "EditorPch.h"

typedef struct tagChangeLevel : public CEvent
{
	LEVEL eNextLevel;
	_bool isLoad;
	tagChangeLevel(LEVEL _eNextLevel, _bool _isLoad)
		: eNextLevel{ _eNextLevel }, isLoad{ _isLoad } {};
}CHANGE_LEVEL_EVENT;

typedef struct tagLoadingEnd : public CEvent
{
	_bool isFinish;
	tagLoadingEnd(_bool _isFinish) : isFinish{ _isFinish } {};
}LOADING_END_EVENT;