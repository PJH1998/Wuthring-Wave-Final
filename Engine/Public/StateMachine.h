#pragma once
#include "Component.h"


NS_BEGIN(Engine)
class ENGINE_DLL CStateMachine : public CComponent
{
#pragma region 기본 함수
public:
	explicit CStateMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CStateMachine(const CStateMachine& Prototype);
	virtual ~CStateMachine() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize_Clone(void* pArg);
#pragma endregion


public:
	virtual void Update(_float fTimeDelta);
	void Change_State(const _string& strStateName);

public:
	void Add_State(const _string& strStateName, class CState* pState);


private:
	map<_string, _uint> m_StateMap; // string, int 키로 저장. => Change_State 할때만 변경.
	vector<class CState*> m_States; // 실제 State 객체들 담아둠.
	_uint m_iCurrentStateIndex = {}; // 현재 실행되는 Index

public:
	static CStateMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END

