#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CLevel_UI final : public CLevel
{
private:
	typedef struct tagHierarchyObjectDesc
	{
		_wstring				strObjName	= {};
		class CCustom_UI*		pCustomUI	= nullptr;
	} HIERARCHY_OBJ_DESC;

private:
	explicit CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_UI() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void		Update(_float fTimeDelta) override;
	virtual void		Render() override;

private:
	void				Update_Picking();

	void				Update_LoadWindow();
	void				Update_Inspector();
	void				Update_Hierarchy();

private:
	vector<HIERARCHY_OBJ_DESC>	m_vecCustomUIs = {};

	class CGameObject*			m_pCurObj = { nullptr };
	class CGameObject*			m_pPreObj = { nullptr };



public:
	static		CLevel_UI*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END