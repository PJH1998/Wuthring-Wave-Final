#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CComputeShader;
NS_END

NS_BEGIN(Client)
class CGameSystem;

class CCoroProduction final : public CGameObject
{
public:
	typedef struct tagCoroProductionDesc : CGameObject::GAMEOBJECT_DESC
	{
		pair<LEVEL, _wstring> shaderData = {};
		pair<LEVEL, _wstring> computeShaderData = {};
		pair<LEVEL, _wstring> modelData = {};
		_string strFolderPath = {};
	}COROPROD_DESC;

private:
	explicit CCoroProduction(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCoroProduction(const CCoroProduction& Prototype);
	virtual ~CCoroProduction() = default;

public:
	virtual	HRESULT Initialize_Prototype() override;
	virtual	HRESULT Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;
	virtual void Render_Shadow() override;

public:
	void Register_AllNotifies(const _string& strFolderPath);

	void Collider_Active(const _wstring&, _bool IsActive);
	void Effect_Active(const _wstring& wStrEffectTag);
	virtual void Object_Func(const _wstring& wStrObjectTag);

private:
	CModel*										m_pModelCom = { nullptr };
	CShader*									m_pShaderCom = { nullptr };
	CComputeShader*								m_pComputeShaderCom = { nullptr };
	CGameSystem*								m_pGameSystem = { nullptr};

	vector<_uint>								m_ShaderIndices;
	_string										m_strCurrentAnimation;
	map<const _string, pair<_float, _float>>	m_Tracks;
	_bool										m_IsPlayAnimation = { false };
	_bool										m_IsRootMotion = { false };
	_bool										m_IsRootMotionRotate = { false };
	_bool										m_IsRootMotionTranslate = { false };
	_bool										m_isActionEnd[3] = {};

private:
	void Bind_Resources();
	HRESULT Ready_Components(const COROPROD_DESC* pDesc);
	void	Action1();
	void	Action2();
	void	Action3();


public:
	virtual	CGameObject* Clone(void* pArg) override;
	static CCoroProduction* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};
NS_END

