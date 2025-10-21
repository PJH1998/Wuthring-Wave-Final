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
		EDIT_ANIMATION = 3, 
		SAVE_STATE = 4,
		END
	};


private:
	explicit CAnimationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAnimationTool() = default;


#pragma region 
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
	void RenderUI_EditState();

private:
	// 2 Depth Menu
	void RenderUI_ConvertFbx();
	void RenderUI_CreateActor();
	void RenderUI_EditAnimation();

	void RenderUI_FromState();
	void RenderUI_ToState();
	void RenderUI_Transitions();
	void RenderUI_OptionState();
	
	

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
	

	list<_string> m_ModelNames;
	list<_string> m_ActorNames;
	list<_string> m_StateTransitions; // Transition 저장할 정보.

	typedef map<const _wstring, class CAnimationActor*> ANIMATIONACTORS;
	ANIMATIONACTORS m_AnimationActors;

	typedef map<const _wstring, const _string> MODELPATHS;
	MODELPATHS	m_ModelDirPaths;

	_wstring m_wSelected_PrototypeModelTag = {};
	_string m_Selected_PrototypeModelTag = {};

	_wstring m_wSelected_AnimActorTag = {};
	_string m_Selected_AnimActorTag = {};


	_string m_SelectedFromStateTag = {};
	_string m_SelectedToStateTag = {};

private:
	_string m_Selected_AnimationTag = {};
	_float m_fTrackPosition = {};
	_float m_fDuration = {};
	_bool m_IsVisibleNotify = { false };
	_bool m_IsPlayAnimation = { true };
	_bool m_IsStateTransition = { false };
	
private:
	_float m_fEditorAlpha = { 1.f };

private:
#ifdef _DEBUG
	void Export_StateAnimationMap_ToCSV();
	void Export_StateTransition_To_CSV();
#endif

private:
	HRESULT Add_Prototype_AnimModel(_wstring strPrototypeName, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);

	

public:
	static CAnimationTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;
};
NS_END

