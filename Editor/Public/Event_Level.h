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

typedef struct tagMapObjectPick : public CEvent
{
	void* pObject = { nullptr };
	_float fDistance;
	tagMapObjectPick(void* _pObject, _float _fDistance) : pObject(_pObject), fDistance(_fDistance) {};
}MAP_PICK;

typedef struct tagMapCreate: public CEvent
{
	_char ModelName[MAX_PATH];
	void* pObject = { nullptr };
	tagMapCreate(_char* _ModelName, void* _pObject) : pObject(_pObject)
	{
		strcpy_s(ModelName, _ModelName);
	};
}MAP_CREATE;

typedef struct tagMapSave : public CEvent
{
	ofstream& File;
	tagMapSave(ofstream& _File) :File(_File){};
}MAP_SAVE;