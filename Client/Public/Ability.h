#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CAbility final : public CComponent
{
public:
	typedef struct tagAbillityDesc
	{
		const _char* pFilePath = {}; // 읽어서 등록할. Ability 파일
	}ABILLITY_DESC;


private:
	explicit CAbility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAbility(const CAbility& Prototype);
	virtual ~CAbility() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize_Clone(void* pArg);
	virtual HRESULT		Render();


private:
	

private:
	void Read_File(const _char* pFilePath);

public:
	static CAbility* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END

