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
// 1. 우선 Subscribe를 통해 EventBus에서 이벤트의 감지 가능하도록. (이벤트가 오기를 대기. 구독)
// 2. 이후 Publish 시 생성자 정의된 event 전달을 통해 변수 할당. 이는 인자와 동일한 역할을 하는 듯 (이벤트를 신청. 발송)

typedef struct tagOnClickEnterUI : public CEvent
{	// m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), L"Event_OnClickEnterUI", ONCLICKENTER_UI_EVENT(iInstanceIndex));
	_uint iInstanceIndex;
	tagOnClickEnterUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONCLICKENTER_UI_EVENT;
typedef struct tagOnClickingUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnClickingUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONCLICKING_UI_EVENT;
typedef struct tagOnClickExitUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnClickExitUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONCLICKEXIT_UI_EVENT;

typedef struct tagOnHoverEnterUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnHoverEnterUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONHOVERENTER_UI_EVENT;
typedef struct tagOnHoveringUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnHoveringUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONHOVERING_UI_EVENT;
typedef struct tagOnHoverExitUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnHoverExitUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONHOVEREXIT_UI_EVENT;
typedef struct tagMinigamePaletteSuccessUI : public CEvent
{
	_bool isSuccess;
	tagMinigamePaletteSuccessUI(_uint iInstanceIndex = 0) : isSuccess{ isSuccess } {};
}MINIGAMEPALETTE_SUCCESS_UI_EVENT;
