#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)
class CAnimMachine final : public CComponent
{
	typedef struct tagAnimMacnineDesc
	{
		_uint*			pOwnerState;
	}ANIMMACNINE_DESC;

private:
	explicit CAnimMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAnimMachine(const CAnimMachine& Prototype);
	virtual ~CAnimMachine() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize_Clone(void* pArg);

	void Handle_Input(CModel* pModelCom, _uint iIndex);
	void Update(_float fTimeDelata, CModel* pModelCom);
	
	void Reset();

private:
	_uint* m_pOwnerState = { nullptr };
	_string m_strCurrentAnimTag;
	vector<class CAnimState*> m_AnimStates;
	_uint m_iCurrentStateIndex{};

public:
	static CAnimMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END
