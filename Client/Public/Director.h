#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CDirector final : public CBase
{
public:
	typedef struct tagCameraAction {

	}CAMERA_ACTION;
private:
	explicit CDirector();
	virtual ~CDirector() = default;

public:
	void		Add_Action(const _char* pFolderPath);
	void		Play_Action(const _wstring& strActionTag, _bool isMaintain); // Tag / true : 유지, false : 끝나면 자동 Recovery

private:
	class CGameInstance*	m_pGameInstance = { nullptr };

	map<const _wstring, CAMERA_ACTION>	m_CameraActions;

public:
	static		CDirector*	Create();
	virtual		void			Free();
};

NS_END