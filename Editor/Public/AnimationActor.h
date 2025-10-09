#pragma once

#include "ContainerObject.h"
NS_BEGIN(Editor)
class CAnimationActor final : public CContainerObject
{
public:
	// 생성시 Shader Tag와 Model Tag 전달.
	typedef struct tagAnimationActorDesc : CContainerObject::GAMEOBJECT_DESC
	{
		LEVEL eLevel = {};
		_wstring strShaderTag = {};
		_wstring strModelTag = {};
		_uint iShaderPath = {};

		// 초기 Transform 설정
		_float3 vPostion = {};
		_float3 vRotation = {};
		_float3 vScale = {};
	}ANIMATION_ACTOR_DESC;

private:
	explicit CAnimationActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAnimationActor(const CAnimationActor& Prototype);
	virtual ~CAnimationActor() = default;

public:
	virtual	HRESULT Initialize_Prototype() override;
	virtual	HRESULT Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;
	virtual void Render_Shadow() override;


#ifdef _DEBUG
public:
	const vector<_string>& Get_AnimationNames() const;
#endif // _DEBUG


	


private:
	LEVEL m_eCurLevel = {LEVEL::END};
	class CModel* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	_uint m_iShaderPath = {};
	_string m_strCurrentAnimation = {};
	_float m_fTrackPosition = {};

private:
	void Bind_Resources();
	HRESULT Ready_Components(const ANIMATION_ACTOR_DESC* pDesc);

public:
	virtual	CGameObject* Clone(void* pArg) override;
	static CAnimationActor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};
NS_END

