#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CStateMachine : public CComponent
{
#pragma region 기본 함수
public:
	explicit CStateMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CStateMachine(const CStateMachine& Prototype);
	virtual ~CStateMachine() = default;

public:
	virtual HRESULT		Initialize_Prototype(const _char* pFilePath);
	virtual HRESULT		Initialize_Clone(void* pArg);
#pragma endregion

public:
	void Change_State(_string strAnimation);

public:
	// 1. 만들어야 하는 기능. Parsing된 데이터 받아오기.
	void Load_Data(const _char* pFilePath);

	// 2. Parsing된 데이터를 받아와서 저장할 멤버변수 생성

private:
	map<_string, _uint> m_StateInfos;
	_uint m_iCurrentStateIndex = {};

	map<_string, class CState*> m_States;

public:
	static CStateMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath) override;
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END

