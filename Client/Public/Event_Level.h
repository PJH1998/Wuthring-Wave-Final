#pragma once
#include "ClientPch.h"

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



// UI Test
// 1. 우선 Subscribe를 통해 EventBus에서 이벤트를 감지 가능하도록.
// 2. 이후 Publish 시 생성자 정의된 event 전달을 통해 변수 할당. 이는 인자와 동일한 역할을 하는 듯
typedef struct tagOnClickUI : public CEvent
{
	
	tagOnClickUI() {};
}ONCLICK_UI_EVENT;

typedef struct tagOnHoverUI : public CEvent
{
	
	tagOnHoverUI() {};
}ONHOVER_UI_EVENT;

typedef struct tagOnScrollUI : public CEvent
{

	tagOnScrollUI() {};
}ONSCROLL_UI_EVENT;