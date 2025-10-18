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
		EDIT_ANIMATION = 3, // ?좊땲硫붿씠???섏젙. (Notify)?
		END
	};


private:
	explicit CAnimationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAnimationTool() = default;


#pragma region 湲곕낯 ?⑥닔
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

	// Prototype????ν븯怨??대쫫留?媛?몄샃?덈떎.
	list<_string> m_ModelNames; // 紐⑤뜽 而댄룷?뚰듃 
	list<_string> m_ActorNames; // ???앹꽦 媛앹껜. 

	typedef map<const _wstring, class CAnimationActor*> ANIMATIONACTORS;
	ANIMATIONACTORS m_AnimationActors;

	typedef map<const _wstring, const _string> MODELPATHS;
	MODELPATHS	m_ModelDirPaths;

	// ?앹꽦??媛앹껜??????숈쟻?쒖뼱瑜??대뼸寃뚰븷源?
	_wstring m_wSelected_PrototypeModelTag = {};
	_string m_Selected_PrototypeModelTag = {};

	_wstring m_wSelected_AnimActorTag = {};
	_string m_Selected_AnimActorTag = {};


private:
	_string m_Selected_AnimationTag = {};
	_float m_fTrackPosition = {};
	_float m_fDuration = {};
	_bool m_IsVisibleNotify = { false };
	_bool m_IsPlayAnimation = { true };
	
private:
	_float m_fEditorAlpha = { 1.f };

private:
	// ?ы띁 ?⑥닔
	HRESULT Add_Prototype_AnimModel(_wstring strPrototypeName, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);

	

public:
	static CAnimationTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;
};
NS_END

