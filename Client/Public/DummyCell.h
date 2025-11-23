#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CDummyCell final : public CGameObject
{
public:
	typedef struct tagDummyCellDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float3 vStartPos = {};
		function<_bool(const _string&, CTransform*, _float, _float*, _bool, _bool, _bool, _float)> pUpdateRootFunc;
		function<void(const _string&, _fmatrix, _uint, _float*, _uint*, _uint)> pUpdateAnimStateFunc;
		_uint iInstanceIndex;
		_uint iNumMeshType;
		_uint* iMeshTypes;
		_float fTrackPos;
		_char szAnimationTag[MAX_PATH];
	}DUMMYCELL_DESC;
private:
	explicit CDummyCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CDummyCell(const CDummyCell& Prototype);
	virtual ~CDummyCell() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() {};

private:
	CCollider* m_pColliderCom = { nullptr };

	_uint m_iInstanceIndex = {};
	function<_bool(const _string&, CTransform*, _float, _float*, _bool, _bool, _bool, _float)> m_pUpdateRootFunc;
	function<void(const _string&, _fmatrix, _uint, _float*, _uint*, _uint)> m_pUpdateAnimStateFunc;
	vector<_uint> m_MeshTypeIndices;

	_string m_strAnimationTag;
	_float m_fTrackPos = {};
	_bool m_isRootMotion{};
	_bool m_isRootMotionRotate{};
	_bool m_isRootMotionTranslate{};
	_float m_fRootMotionRate{};
	_uint m_iFaceIndex{};
private:
	void Ready_Component(DUMMYCELL_DESC* pDesc);

public:
	static CDummyCell* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
