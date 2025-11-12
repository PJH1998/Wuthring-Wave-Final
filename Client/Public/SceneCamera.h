#pragma once
#include "Camera.h"

NS_BEGIN(Client)

class CSceneCamera final : public CCamera
{
private:
	explicit CSceneCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSceneCamera(const CSceneCamera& Prototype);
	virtual ~CSceneCamera() = default;

public:
	virtual		HRESULT				Initialize_Prototype() override;
	virtual		HRESULT				Initialize_Clone(void* pArg) override;
	virtual		void					Priority_Update(_float fTimeDelta) override;
	virtual		void					Update(_float fTimeDelta) override;
	virtual		void					Late_Update(_float fTimeDelta) override;
	virtual		void					Render() override;

	virtual		void					Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	vector<SCENE_CAMERA_FRAME>		m_Frames;
	_int											m_iFrameIndex = { 0 };
	_float											m_fTrackPosition = {};
	_float											m_fTrackPerSec = { 10.f };

	_float											m_fStartFrame = {};
	_float											m_fEndFrame = {};

	_float											m_fVelocity = {};
	_float											m_fSpeed = {};

	_float											m_fRatio = {};

private:
	void								Default_SetUp();
	void								Lerp_Quat();
	void								Spline();

public:
	static		CSceneCamera*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END