#pragma once
#include "Level.h"

NS_BEGIN(Client)

class CLevel_Test :
    public CLevel
{
private:
	explicit CLevel_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Test() = default;

public:
	virtual		HRESULT			Initialize() override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Render() override;

private:
	HRESULT Ready_Layer_Map(const _char* pFilePath);
	void Read_Map_Dat(const _string pFilePath);
	void Ready_Layer_Player();

private:
	LEVEL m_eCurLevel = { LEVEL::TEST };
	class CGameSystem* m_pGameSystem = { nullptr };

public:
	static		CLevel_Test* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END