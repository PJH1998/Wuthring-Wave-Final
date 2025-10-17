#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
	typedef struct tagCameraDesc : public CGameObject::GAMEOBJECT_DESC {
		_float4	vEye{}, vAt{};
		_float		fFovy{};
		_float		fNear{}, fFar{};
		_float		fMouseSensor{};
	}CAMERA_DESC;
protected:
	explicit CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCamera(const CCamera& Prototype);
	virtual ~CCamera() = default;

public:
	void							Set_Distance(_float fDistance) { m_fDistance += fDistance; }
	_float							Get_Distance() { return m_fDistance; }
	void							Set_FixedDistance(_float fFixedDistance) { m_fFixedDistance = fFixedDistance; }
	_float							Get_Near() { return m_fNear; }
	_float							Get_Far() { return m_fFar; }
#ifdef _DEBUG
	_float*							Get_DistancePtr() { return &m_fFixedDistance; }
#endif // _DEBUG


public:
	virtual		HRESULT			Initialize_Prototype();
	virtual		HRESULT			Initialize_Clone(void* pArg);
	virtual		void				Priority_Update(_float fTimeDelta);
	virtual		void				Update(_float fTimeDelta);
	virtual		void				Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta);
	virtual		void				Late_Update(_float fTimeDelta);
	virtual		void				Render();

	void							Update_Matrix();

protected:
	_float							m_fFovy = {};
	_float							m_fAspect = {};
	_float							m_fNear{}, m_fFar{};
	_float							m_fMouseSensor{};

	_float							m_fDistance = {};
	_float							m_fFixedDistance = {};
	_float							m_fLerpSpeed = {};

protected:
	void							Lerp_Distance(_float fTimeDelta);
	void							Key_Move(_float fTimeDelta);
	void							Mouse_Move();
	void							Mouse_Move_Up();

public:
	virtual CGameObject*		Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END