#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)
class CGgobul final : public CGameObject
{
public:
	enum GGOBULTYPE { HEAD, HAMMER, KNIFE, END};
	typedef struct tagGgobulDesc : public CGameObject::GAMEOBJECT_DESC
	{
		pair<_uint, _wstring> shaderData;
		pair<_uint, _wstring> computeShaderData;
		pair<_uint, _wstring> modelData;
		pair<_uint, _wstring> rigidBodyData;
		_string strFolderPath;
	}GGOBUL_DESC;

	typedef struct tagGgobulReset
	{
		_float4x4* pWorldMatrix;
		GGOBULTYPE eType;
		_float fChangeTrackPos;
		_string strPatternKey;

		_bool isRootMotion;
		_bool isRootMotionRotate;
		_bool isRootMotionTranslate;
		_float fRootMotionRate;
		_float fAnimationSpeed;
	}GGOBUL_RESET;

private:
	explicit CGgobul(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CGgobul(const CGgobul& Prototype);
	virtual ~CGgobul() = default;

public:
	virtual	HRESULT				Initialize_Prototype() override;
	virtual	HRESULT				Initialize_Clone(void* pArg) override;
	virtual	void				Priority_Update(_float fTimeDelta) override;
	virtual	void				Update(_float fTimeDelta) override;
	virtual	void				Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg);

private:
	class CComputeShader*	m_pComputeShaderCom = { nullptr };
	class CShader*			m_pShaderCom = { nullptr };
	class CModel*			m_pModelCom = { nullptr };
	class CRigidbody*		m_pRigidBodyCom[3] = {nullptr,};

	GGOBULTYPE				m_eType{};
	const _float4x4*		m_pAttackTransform = { nullptr };
	_float4x4				m_CombindMatrix{};
	vector<_bool>			m_MeshEnables;

	_string m_strAnimKey;

	_bool m_isRootMotion;
	_bool m_isRootMotionRotate;
	_bool m_isRootMotionTranslate;
	_float m_fRootMotionRate;
	_float m_fAnimationSpeed;

private:
	void			Bind_Resources();
	void			Ready_Component(GGOBUL_DESC* pDesc);
	void			Register_AllNotifies(const _string& strFolderPath);
	virtual void	Collider_Active(const _wstring& wStrColliderTag, _bool Isactive);
	virtual void	Effect_Active(const _wstring& wStrEffectTag);
	void			OnCollide_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
public:
	static CGgobul* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) override;
	virtual	void Free() override;
};
NS_END
