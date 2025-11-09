#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CRigidbody;
class CCollider;
NS_END

NS_BEGIN(Client)

class CPatternDummy final : public CGameObject
{
public:
	typedef struct tagPatternDummyDesc
	{
		LEVEL eLevel;
		_float3 vInitPosition;
		_wstring strModelTag;
		_string strInitAnimTag;
		_string strFolderPath;
	}PAT_DUMMYDESC;
private:
	explicit CPatternDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CPatternDummy(const CPatternDummy& Prototype);
	virtual ~CPatternDummy() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;

	void						OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void						OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void						OnCollide_Remove(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

private:
	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	//CRigidbody*				m_pRigidbodyCom = { nullptr };
	CCollider*				m_pColliderCom = { nullptr };

	_string					m_strAnimTag;
	_string					m_strInitAnimTag;
	_bool					m_isRootMotion{true};
	_bool					m_isRootRotate{};
	_bool					m_isRootTranslate{};
	_float3					m_vPosition{};
#ifdef _DEBUG
	vector<_string>			m_strAnimationTags;
#endif // _DEBUG
	_float					m_fTrackPosition{};

private:
	void						Ready_Component(PAT_DUMMYDESC* pDesc);
	void						Register_AllNotifies(const _string& strFolderPath);
	void						Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) {};
	void						Effect_Active(const _wstring& wStrEffectTag);
	void						Object_Func(const _wstring& wStrObjectTag) {};

public:
	static		CPatternDummy*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END