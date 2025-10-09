#pragma once
#include "Base.h"

NS_BEGIN(Editor)
// ImGui Animation Tool
class CAnimationTool final : public CBase
{
private:
	enum class MODE : _uint
	{
		CONVERT_FBX_TO_DAT = 0, 
		LOAD_DAT = 1, 
		CREATE_ACTOR = 2,
		EDIT_ANIMATION = 3, // 애니메이션 수정. (Notify)?
		END
	};


private:
	explicit CAnimationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAnimationTool() = default;


#pragma region 기본 함수
public:
	HRESULT	Initialize(LEVEL eLevel);
	void Update();
	void Render();
#pragma endregion


private:
	// 1 Depth Menu
	void Render_Editor();
	void Render_DebugWindow();
	void Render_Menu();


private:
	// 2 Depth Menu
	void RenderUI_ConvertFbx();
	void RenderUI_CreateActor();
	void RenderUI_EditAnimation();

private:
	// 3 Depth Menu
	void LoadDat();
	void RenderUI_ModelPrototype();
	void RenderUI_AnimationList();

private:
	// 4 Depth Menu
	void Render_Model_Detail();
	void Render_Animation_Detail();

	
	

private:
	LEVEL m_eCurLevel = { LEVEL::END };
	MODE m_eMode = { MODE::END };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	class CModelLoader* m_pLoader = { nullptr };
	class CAnimNotifyTool* m_pAnimNotifyTool = { nullptr };

	// Prototype에 저장하고 이름만 가져옵니다.
	list<_string> m_ModelNames; // 모델 컴포넌트 
	list<_string> m_ActorNames; // 실 생성 객체. 

	typedef map<const _wstring, class CAnimationActor*> ANIMATIONACTORS;
	ANIMATIONACTORS m_AnimationActors;

	// 생성한 객체에 대한 동적제어를 어떻게할까?
	_wstring m_wSelected_PrototypeModelTag = {};
	_string m_Selected_PrototypeModelTag = {};

	_wstring m_wSelected_AnimActorTag = {};
	_string m_Selected_AnimActorTag = {};


private:
	_string m_Selected_AnimationTag = {};
	_float m_fTrackPosition = {};
	_float m_fDuration = {};
	
private:
	_float m_fEditorAlpha = { 1.f };

private:
	// 헬퍼 함수
	wstring StringToWstring(const std::string& str);
	string WstringToString(const std::wstring& wstr);
	HRESULT Add_Prototype_AnimModel(_wstring strPrototypeName, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);

	

public:
	static CAnimationTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;
};
NS_END

