#pragma once
#include "ClientPch.h"

typedef struct tagCameraAction : public CEvent
{
	vector<CAMERA_FRAME>&		pFrame;
	_bool									isAction = { false };
	_int									iStart = {};
	_int									iEnd = {};
	_float4x4								WorldMatrix = {};
	_bool									isMaintain = { false };
	_bool									isEscape = { false };
	tagCameraAction(vector<CAMERA_FRAME>& _Frame, _bool _isAction, _int _iStart, _int _iEnd, const _float4x4& _WorldMatrix, _bool _isMaintain, _bool _isEscape = false)
		: pFrame{ _Frame }, isAction{ _isAction }, iStart{ _iStart }, iEnd{ _iEnd },
		WorldMatrix { _WorldMatrix }, isMaintain { _isMaintain }, isEscape { _isEscape }
	{
	};
}CAMERA_ACTION_EVENT;