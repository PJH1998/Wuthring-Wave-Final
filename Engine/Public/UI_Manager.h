#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CUI_Manager final : public CBase
{
private:
	explicit	CUI_Manager();
	virtual		~CUI_Manager() = default;

public:
	HRESULT		Initialize();
	void		Update(_float fTimeDelta);				// 마우스의 위치 여부를 감지하고, 현재 UI들 위에 있는지, 위에 있다면 hover, 클릭했다면 click 등의 이벤트 확인용 변수를 전환한다.

	HRESULT		Add_RootUI(class CUIObject* rootUI);
	void		Clear_RootUI();

private:
	vector<class CUIObject*>	m_vecRootUIs = {};

public:
	static		CUI_Manager*	Create();
	virtual		void			Free() override;

};

NS_END	