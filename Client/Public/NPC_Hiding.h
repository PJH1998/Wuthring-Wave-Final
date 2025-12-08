#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CAnimMachine;
NS_END

NS_BEGIN(Client)
class CGameSystem;

class CNPC_Hiding final : public CActor
{
public:
	typedef struct tagHidingDesc : public CActor::ACTOR_DESC
	{
		_float3 vInitPos;
		_float3 vInitRot;
		_bool isCollide;
		const _char* pAnimationTag;
		const _tchar* pAnimMachineTag;
	}HIDINGDESC;
	enum MESH_TYPE { FACE, HAIR, BODY, END };

private:
	explicit CNPC_Hiding(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CNPC_Hiding(const CNPC_Hiding& Prototype);
	virtual ~CNPC_Hiding() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual	void	Render_Shadow() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;

public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
	virtual void Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void Object_Func(const _wstring& wStrObjectTag) override; // 임시

private:
	CAnimMachine*			m_pAnimMachineCom = { nullptr };
	CGameSystem*			m_pGameSystem = { nullptr };

	_bool					m_isRender{};
	_bool					m_isDesolve{};
	_bool					m_isScaned{};
	_bool					m_isFind{};
	_bool					m_isReturn{};
	_float					m_fScanAcc{};
	_float					m_fScanRate{};
	_float					m_fDesolveRate{};
	_float4					m_vBaseColor{};
	_uint					m_iFaceIndex{};
	_uint					m_iState{};

private:
	HRESULT		Bind_Resources();
	void		Ready_Component(HIDINGDESC* pDesc);
	void		OnDetect_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void		OnDetect_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void		OnDetect_Remove(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

	void		OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static CNPC_Hiding* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END
