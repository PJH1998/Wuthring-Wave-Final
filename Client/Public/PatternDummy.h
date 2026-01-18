#pragma once
#include "ContainerObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CRigidbody;
class CCollider;
NS_END

NS_BEGIN(Client)

class CPatternDummy final : public CContainerObject
{
public:
	enum MODEL_TYPES {DEFAULT, WEAPON};
	typedef struct tagPatternDummyDesc : public CContainerObject::GAMEOBJECT_DESC
	{
		LEVEL eLevel;
		_float3 vInitPosition;
		_float3 vInitRotation;
		_wstring strModelTag;
		_wstring strPartTag;
		_string strBoneName;
		_string strInitAnimTag;
		_float3 vOffsetPos;
		_float3 vOffsetRot;
		_string strFolderPath;
		MODEL_TYPES eType;
		_bool isCollide{ false };
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
	CShader*					m_pShaderCom = { nullptr };
	CComputeShader*				m_pComputeShaderCom = { nullptr };
	//CComputeShader*				m_pFacialShaderCom = { nullptr };
	CModel*						m_pModelCom = { nullptr };
	//CRigidbody*				m_pRigidbodyCom = { nullptr };
	CCollider*					m_pColliderCom = { nullptr };
	CALLBACK_CLIENT				m_tCallBack{};
	_string					m_strAnimTag;
	_string					m_strInitAnimTag;
	_bool					m_isRootMotion{true};
	_bool					m_isRootRotate{};
	_bool					m_isRootTranslate{};
	_float3					m_vPosition{};
	MODEL_TYPES				m_eType{};
#ifdef _DEBUG
	vector<_string>			m_strAnimationTags;
#endif // _DEBUG
	_float					m_fTrackPosition{};

private:
	void						Ready_Component(PAT_DUMMYDESC* pDesc);
	void						Ready_PartObjects(PAT_DUMMYDESC* pDesc);
	void						Register_AllNotifies(const _string& strFolderPath);
	void						Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) {};
	void						Effect_Active(const _wstring& wStrEffectTag);
	void						Object_Func(const _wstring& wStrObjectTag);

public:
	static		CPatternDummy*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END