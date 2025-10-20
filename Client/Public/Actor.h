#pragma once
#include "ContainerObject.h"

NS_BEGIN(Client)
class CActor abstract : public CContainerObject
{


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

public:

protected:


public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;
};
NS_END

