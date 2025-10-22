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
// 1. �켱 Subscribe�� ���� EventBus���� �̺�Ʈ�� ���� �����ϵ���.
// 2. ���� Publish �� ������ ���ǵ� event ������ ���� ���� �Ҵ�. �̴� ���ڿ� ������ ������ �ϴ� ��
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