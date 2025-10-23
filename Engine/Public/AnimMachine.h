#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class CModel;

class ENGINE_DLL CAnimMachine final : public CComponent
{
public:
	typedef struct tagAnimMacnineDesc
	{
		const _char* pAnimationTag;
	}ANIMMACNINE_DESC;

	typedef struct tagTransitionData
	{
		_string strFrom;
		_string strTo;
		_uint iTargetState;
		_float fTransitEnablePos;
		//_uint iNextStateIndex; strTo에서 직접 찾기

	}TRANSITION_DESC;

private:
	explicit CAnimMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAnimMachine(const CAnimMachine& Prototype);
	virtual ~CAnimMachine() = default;

public:
	virtual HRESULT		Initialize_Prototype(/*const _char* AnimMachineDataPath*/) override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

	//void Handle_Input(CModel* pModelCom, _uint* pState, _uint iIndex);
	void Handle_Input(CModel* pModelCom, _uint* pState,_string strAnimTag);
	void Update(CModel* pModelCom, _uint* pState, _float fTimeDelata);
	
	void Reset();

private:
	_string m_strCurrentAnimTag;
	//vector<class CAnimState*> m_AnimStates;
	map<_string, class CAnimState*> m_AnimStates;
	_uint m_iCurrentStateIndex{};

public:
	static CAnimMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext /*, const _char* AnimMachineDataPath*/);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END
