#pragma once

#include "ContainerObject.h"
NS_BEGIN(Editor)
class CAnimationActor final : public CContainerObject
{
public:
	// ?앹꽦??Shader Tag? Model Tag ?꾨떖.
	typedef struct tagAnimationActorDesc : CContainerObject::GAMEOBJECT_DESC
	{
		LEVEL eLevel = {};
		_wstring strShaderTag = {};
		_wstring strComputeShaderTag = {};
		_wstring strModelTag = {};
		_string strModelDatPath = {};

		_uint iShaderPath = {};

		// 珥덇린 Transform ?ㅼ젙
		_float3 vPostion = {};
		_float3 vRotation = {};
		_float3 vScale = {};

		// 臾닿린???μ갑 媛?ν븯寃?..
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
	_float* Get_TrackPositionPtr(const _string& strAnimName);
	_float	Get_Duration(const _string& strAnimName);

	const _string& Get_CurrentAnimationNames() const;
	const _float Get_CurrentAnimationDuration() const;

	//Bone GUI Ȱ��ȭ
	HRESULT Bind_Bone_to_GUI();

	void Change_CurrentAnimation(_string strAnimName) { m_strCurrentAnimation = strAnimName; }
	void Set_TrackPosition(_float fTrackPosition);
	void Set_PlayAnimation(_bool IsPlay);

	void Register_AllNotifies(const _string& strFolderPath);


	// ?뚯뒪??肄쒕갚 ?⑥닔.
	void Collider_Active(const _wstring&, _bool IsActive);
	void Effect_Active();
#endif // _DEBUG


private:
	LEVEL m_eCurLevel = {LEVEL::END};
	class CModel* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CComputeShader* m_pComputeShaderCom = { nullptr };

	_uint m_iShaderPath = {};
	_string m_strCurrentAnimation = {};
	_string m_strCurrentRibAnimation = {};
	_float m_fTrackPosition = {};
	_bool m_IsPlayAnimation = { true };

	_float m_fTimeDelta = {}; // Stop ?곹깭?먯꽌??PlayAnimation ?숈옉???꾪븿.

	_string m_strModelDatPath = {}; // 

private:
	void Bind_Resources();
	HRESULT Ready_Components(const ANIMATION_ACTOR_DESC* pDesc);

public:
	virtual	CGameObject* Clone(void* pArg) override;
	static CAnimationActor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};
NS_END

