#pragma once
#include "Base.h"

NS_BEGIN(Editor)

// Notify를 생성하고 불러올 수 있는 Tool
class CAnimNotifyTool final : public CBase
{
public:
	enum class NOTIFYTYPE : _uint
	{
		COLLIDER = 0, // 애니메이션 콜라이더 활성화.
		EFFECT = 1,   // EFFECT 
		SOUND = 2,	  // SOUND
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
	void Process_Notify(class CAnimationActor* pActor, const _string& strAnimName, _float fTrackPosition);
#pragma endregion


private:
	// Depth1
	void RenderUI_EditNotify();


private:
	// Depth2
	void RenderUI_EditSound();
	void RenderUI_EditEffect();
	void RenderUI_EditCollider();
	

private:
	// Depth3
	void Load_SoundFiles();
	void Edit_SoundNotify();

private:
	// Depth4
	void Load_SoundsFromFile(const _string& strFilePath, const _string& strSoundPath);
	void Load_AllSoundsFromFolder(const _string& strFolderPath);

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	LEVEL m_eCurLevel = { LEVEL::END };
	NOTIFYTYPE m_eType = { NOTIFYTYPE::END };

	// Notify 등록 시 무조건 필요한 정보
	class CAnimationActor* m_pCurrentActor = { nullptr };
	_string m_CurrentAnimName = {};
	_float m_fCurrentDuration = {};

	// Sound Tag
	map<const _string, const _wstring> m_SoundTags = {};
	_string m_CurrentSoundTag = {};
private:
	HRESULT Ready_Sound();


private:
	// 헬퍼 함수
	wstring StringToWstring(const std::string& str);
	string WstringToString(const std::wstring& wstr);

public:
	static CAnimNotifyTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;

};
NS_END

