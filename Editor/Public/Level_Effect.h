#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CLevel_Effect final : public CLevel
{
private:
	explicit CLevel_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Effect() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;


private:
	class CEffect_Controller*										m_pEffect_Controller = { nullptr };
	//
	class CAnimationTool*											m_pAnimation_Tool = { nullptr };

public:
	static		CLevel_Effect*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END