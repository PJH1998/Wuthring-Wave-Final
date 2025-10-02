#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CPartObject abstract : public CGameObject
{
public:
	typedef struct tagPartObjectDesc : public CGameObject::GAMEOBJECT_DESC {
		class CTransform* pParentTransform = {nullptr};
		class CNavigation* pParentNavigation = { nullptr };
		_uint*					pActionState = { nullptr };
		_uint*					pSubActionState = { nullptr };
		_float*				pTrackPosition = { nullptr };
	} PART_DESC;
protected:
	explicit CPartObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CPartObject(const CPartObject& Prototype);
	virtual ~CPartObject() = default;

public:
	//void								Set_KeyState(_uint iKeyState) { m_iKeyState = iKeyState; }
	const _float4x4*				Get_SocketBoneMatrix(const _char* pBoneName);
	const _float4x4*				Get_CombinedMatrix() { return &m_CombinedMatrix; }

public:
	virtual		HRESULT				Initialize_Prototype();
	virtual		HRESULT				Initialize_Clone(void* pArg);
	virtual		void					Priority_Update(_float fTimeDelta);
	virtual		void					Update(_float fTimeDelta);
	virtual		void					Late_Update(_float fTimeDelta);
	virtual		HRESULT				Render();

protected:
	class CShader*					m_pShaderCom = { nullptr };
	class CModel*					m_pModelCom = { nullptr };

	class CTransform*				m_pParentTransform = { nullptr };
	class CNavigation*				m_pParentNavigation = { nullptr };
	_uint*								m_pActionState = { nullptr };
	_uint*								m_pSubActionState = { nullptr };
	_float*							m_pTrackPosition = { nullptr };

	_float4x4							m_CombinedMatrix = {};

public:
	virtual		CGameObject*		Clone(void* pArg) = 0;
	virtual		void					Free() override;
};

NS_END