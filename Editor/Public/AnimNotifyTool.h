#pragma once
#include "Base.h"
#include "AnimNotifyDefine.h"

NS_BEGIN(Editor)

// Notify를 생성하고 불러올 수 있는 Tool
class CAnimNotifyTool final : public CBase
{
public:
	enum class NOTIFYTYPE : _uint
	{
		SOUND = 0,	  // SOUND
		EFFECT = 1,   // EFFECT 
		COLLIDER = 2, // 애니메이션 콜라이더 활성화.
		LIGHT = 3,    // LIGHT?
		END
	};

private:
	explicit CAnimNotifyTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAnimNotifyTool() = default;



#pragma region 기본 함수
public:
	HRESULT	Initialize(LEVEL eLevel);
	void Update();
	void Render();
#pragma endregion


#pragma region ANIMATION Tool로부터 받을 정보.
public:
	void Process_Notify(const _string& strAnimName, const _string& strModelDatPath, _float fTrackPosition);
	
#pragma endregion

public:
	void Clear();

private:
	// Depth1
	void RenderUI_EditNotify();


private:
	// Depth2
	void RenderUI_EditSound();
	void RenderUI_EditEffect();
	void RenderUI_EditCollider();
	void RenderUI_SaveNotify();
	void RenderUI_LoadNotify();
	

private:
	// Depth3
	void Load_SoundFiles();
	void Select_SoundNotify();

	void Render_CurrentNotify();
	void Save_NotifyToJson();
	void Load_NotifyFromJson();


private:
	// Depth4
	void Load_SoundsFromFile(const _string& strFilePath, const _string& strSoundPath);
	void Load_AllSoundsFromFolder(const _string& strFolderPath);
	
	void Edit_SoundNotify();


	

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	LEVEL m_eCurLevel = { LEVEL::END };
	NOTIFYTYPE m_eType = { NOTIFYTYPE::END };

	// Notify 등록 시 무조건 필요한 정보
	_string m_strCurrentAnimName = {};
	_string m_strCurrentFolderPath = {};
	_float m_fCurrentDuration = {};

	// Sound Tag
	map<const _string, const _wstring> m_SoundTags = {};
	_string m_CurrentSoundTag = {};


private:
	// Save 용도 변수들
	list<SOUNDNOTIFY>    m_SoundNotifyes;
	list<EFFECTNOTIFY>   m_EffectNotifyes;
	list<COLLIDERNOTIFY> m_ColliderNotifyes;
	list<LIGHTNOTIFY>    m_LightNotifyes;

private:
	HRESULT Ready_Sound();


public:
	static CAnimNotifyTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;

};
NS_END

