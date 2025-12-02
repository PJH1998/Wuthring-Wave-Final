#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CDirector final : public CBase
{
public:
	typedef struct tagCameraAction {
		_int iFrameStart{}, iFrameEnd{};
		vector<CAMERA_FRAME> Frames;
	}CAMERA_ACTION;
private:
	explicit CDirector();
	virtual ~CDirector() = default;

public:
	void		Add_Action(const _char* pFolderPath);
	void		Play_Action(const _wstring& strActionTag, const _fmatrix& WorldMatrix, _bool isMaintain, _bool isEscape); // Tag / true : 유지, false : 끝나면 자동 Recovery
	void		Stop_Action();
	void		Clear_Action();

private:
	class CGameInstance*	m_pGameInstance = { nullptr };

	map<const _wstring, CAMERA_ACTION>	m_CameraActions;

private:
	const CAMERA_ACTION& Find_Action(const _wstring& strActionTag);

public:
	static		CDirector*	Create();
	virtual		void			Free();
};

NS_END