#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CModelAnim_Instance;
class CComputeShader;
NS_END

NS_BEGIN(Client)
class CDummyNPC final : public CActor
{
public:
	enum MESHTYPE { BODY, FACE, HAIR, END };

	typedef struct tagDummyNPCDesc : public CActor::ACTOR_DESC
	{
		_float3 vStartPositions;
		_wstring wstrObjectPrototypeTag;
		_wstring wstrSkinningPrototypeTag;

	}DUMMYNPC_DESC;

private:
	explicit CDummyNPC(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CDummyNPC(const CDummyNPC& Prototype);
	virtual ~CDummyNPC() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;


public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
	virtual void Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void Object_Func(const _wstring& wStrObjectTag) override; // 임시
	virtual void Hit_Judge(void* pArg = nullptr) {};// 임시

private:
	CModelAnim_Instance*	m_pModelInstanceCom = { nullptr };
	CComputeShader*			m_pSkinningCom = { nullptr };
	vector<_uint>			m_MeshTypePadding;

	_float					m_fFaceSize{};
	_uint					m_iFacePaddingCount{};
private:
	HRESULT		Bind_Resources();
	void		Ready_Component(DUMMYNPC_DESC* pDesc);
	void		Ready_InstanceCells(DUMMYNPC_DESC* pDesc);

public:
	static CDummyNPC* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
