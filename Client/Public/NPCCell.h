#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CCollider;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CNPCCell final : public CGameObject
{
public:
	typedef struct tagDummyCellDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float3 vStartPos;
		_float3 vStartRot;
		function<_bool(const _string&, CTransform*, _float, _float*, _bool, _bool, _bool, _float)> pUpdateRootFunc;
		function<void(const _string&, _fmatrix, _uint, _float*, _uint*, _uint)> pUpdateAnimStateFunc;
		_uint iInstanceIndex;
		_uint iNumMeshType;
		_uint* iMeshTypes;
		_uint iFaceIndex;
		_float fTrackPos;
		_char szAnimationTag[MAX_PATH];
		_bool isCollide;
		_bool isRigid;
	}DUMMYCELL_DESC;
private:
	explicit CNPCCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CNPCCell(const CNPCCell& Prototype);
	virtual ~CNPCCell() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() {};

private:
	CCollider* m_pColliderCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };

	_uint m_iInstanceIndex = {};
	function<_bool(const _string&, CTransform*, _float, _float*, _bool, _bool, _bool, _float)> m_pUpdateRootFunc;
	function<void(const _string&, _fmatrix, _uint, _float*, _uint*, _uint)> m_pUpdateAnimStateFunc;
	vector<_uint> m_MeshTypeIndices;

	_string m_strAnimationTag;
	_string m_strOriginAnimationTag;
	_float m_fTrackPos = {};
	_bool m_CollideTrigger{};
	_bool m_isRootMotion{};
	_bool m_isRootMotionRotate{};
	_bool m_isRootMotionTranslate{};
	_float m_fRootMotionRate{};
	_uint m_iFaceIndex{};
	_uint m_iOriginFaceIndex{};
	_float3 m_vDetectOffset{};
	CALLBACK_CLIENT m_tCallBack{};

private:
	void Ready_Component(DUMMYCELL_DESC* pDesc);
	void OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static CNPCCell* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
