#pragma once
#include "Actor.h"
NS_BEGIN(Engine)
class CModelAnim_Instance;
class CComputeShader;
NS_END

NS_BEGIN(Client)
class CNPCInstancing final : public CActor
{
public:
	enum MESHTYPE {BODY, FACE, HAIR, END};

	typedef struct tagNPCDesc : public CActor::ACTOR_DESC
	{
		_wstring wstrObjectPrototypeTag;
		_wstring wstrCombiningPrototypeTag;
		vector<_float3> vStartPositions;
		vector<_string> InitAnimations;
		vector<vector<_uint>> MeshTypes;

	}NPC_DESC;

private:
	explicit CNPCInstancing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CNPCInstancing(const CNPCInstancing& Prototype);
	virtual ~CNPCInstancing() = default;

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
	CModelAnim_Instance* m_pModelInstanceCom = { nullptr };
	CComputeShader* m_pSkinningCom = { nullptr };
	vector<_uint>			m_MeshTypePadding;

	_float					m_fFaceSize{};
	_uint					m_iFacePaddingCount{};
private:
	HRESULT		Bind_Resources();
	void		Ready_Component(NPC_DESC* pDesc);
	void		Ready_InstanceCells(NPC_DESC* pDesc);

	void Update_AnimationState(const _string& strAnimName, _fmatrix WorldMatrix, _uint iInstanceIndex, _float* pTrackPos, _uint* pPaddingIndices);

public:
	static CNPCInstancing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
