#pragma once
#include "ContainerObject.h"

NS_BEGIN(Client)
class CActor abstract : public CContainerObject
{
public:
	typedef struct tagActorDesc : public CGameObject::GAMEOBJECT_DESC
	{
		LEVEL eCurLevel = { LEVEL::END };
		_wstring strShaderTag = {};
		_wstring strComputeShaderTag = {};
		_wstring strModelTag = {};
		_uint iShaderPath;
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


#pragma endregion


protected:
	class CModel* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CComputeShader* m_pComputeShaderCom = { nullptr };
	LEVEL m_eCurLevel = { LEVEL::END };
	_uint m_iShaderPath = {};
	_float m_fTrackPosition = {};


private:
	HRESULT Ready_Components(const ACTOR_DESC* pDesc);

public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;
};
NS_END

