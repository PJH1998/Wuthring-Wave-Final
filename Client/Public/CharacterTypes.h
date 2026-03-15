#pragma once

typedef struct tagHitDesc {
	_uint iLayer;
	_float fAttack;
	CTransform* pTransform = { nullptr };
	_bool IsBack = { false };
}HIT_DESC;

typedef struct tagParryDesc {
	_uint iLayer;
	_float fAttack;
	CTransform* pTransform = { nullptr };
}PARRY_DESC;

typedef struct tagCaptureDesc {
	_uint iLayer;
	_float fAttack;
	CTransform* pTransform = { nullptr };
	const _float4x4* pSocketMatrix = { nullptr };
}CAPTURE_DESC;

typedef struct tagQTEDesc {

	CTransform* pTargetTransform = { nullptr };
	CHARACTER_EVENT eEvent = { CHARACTER_EVENT::END };
}QTE_DESC;

typedef struct tagDelayedAction {
	enum class TYPE { HIT, PARRY, DODGE, GRAB }; // 이벤트 타입.
	TYPE type;

	HIT_DESC hitDesc{};
	CAPTURE_DESC captureDesc{};
}DELAYED_ACTION;