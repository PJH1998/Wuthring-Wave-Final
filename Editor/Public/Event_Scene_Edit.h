#pragma once
#include "EditorPch.h"

typedef struct tagCameraAction : public CEvent
{
	vector<CAMERA_FRAME>&		pFrame;
	_bool									isAction = { false };
	_int									iStart = {};
	_int									iEnd = {};
	tagCameraAction(vector<CAMERA_FRAME>& _pFrame, _bool _isAction, _int _iStart, _int _iEnd)
		: pFrame{ _pFrame }, isAction{ _isAction }, iStart{ _iStart }, iEnd{ _iEnd }
	{};
}CAMERA_ACTION_EVENT;