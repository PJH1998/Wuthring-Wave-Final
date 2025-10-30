#pragma once
#include "Client_Define.h"
#include "ContainerObject.h"

NS_BEGIN(Client)
class CActor abstract : public CContainerObject
{
public:
	typedef struct tagActorDesc : public CGameObject::GAMEOBJECT_DESC
	{
		LEVEL eCurLevel = { LEVEL::END };
		pair<LEVEL, _wstring> shaderData = {};
		pair<LEVEL, _wstring> computeShaderData = {};
		pair<LEVEL, _wstring> colliderData = {};
		pair<LEVEL, _wstring> rigidBodyData = {};
		pair<LEVEL, _wstring> modelData = {};
		_string strFolderPath = {};
	}ACTOR_DESC;


#pragma region 기본 함수
protected:
	explicit CActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CActor(const CActor& Prototype);
	virtual ~CActor() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;


public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool Isactive) {};
	virtual void Effect_Active(const _wstring& wStrEffectTag) {};
	virtual void Hit_Judge() {};



#pragma endregion

protected:
	class CModel* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CComputeShader* m_pComputeShaderCom = { nullptr };
	class CCollider* m_pColliderCom = { nullptr };
	class CRigidbody* m_pRigidBodyCom = { nullptr };
	vector<_uint> m_ShaderPaths = {}; 
	LEVEL m_eCurLevel = { LEVEL::END };
	_float m_fTrackPosition = {};

protected:
	void Register_AllNotifies(const _string& strFolderPath);
	// void Ready_Components(const ACTOR_DESC* pDesc);


public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;
};
NS_END

