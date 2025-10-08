#pragma once
#include "Base.h"

NS_BEGIN(Editor)
// ImGui Animation Tool
class CAnimationTool final : public CBase
{
private:
	enum class MODE
	{
		CONVERT_FBX_TO_DAT = 0, 
		VIEW_DAT = 1, 
		EDIT_ANIMATION = 2, // 애니메이션 수정. (Notify)?
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
	void Render_SelectMode();
	void Render_Menu();


private:
	// 2 Depth Menu
	void RenderUI_ConvertFbx();
	void RenderUI_ViewDat();
	void RenderUI_EditAnimation();

private:
	// 3 Depth Menu
	void LoadDat();
	void RenderUI_Prototype();


private:
	// 4 Depth Menu
	HRESULT Add_Prototype_AnimModel(_wstring strPrototypeName, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);
	void Render_Model_Inspector();
	

private:
	LEVEL m_eCurLevel = { LEVEL::END };
	MODE m_eMode = { MODE::END };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	class CModelLoader* m_pLoader = { nullptr };

	// 아. ImGui string밖에 안됨.
	list<_string> m_ModelNames; // 생성된 Model Component들. => .dat를 읽어와서 저장합니다.
	list<_string> m_ActorNames; // 생성된 GameObject들.

	_wstring m_wSelected_PrototypeModelTag = {};
	_string m_Selected_PrototypeModelTag = {};

	_float m_fEditorAlpha = { 1.f };
	



private:
	// 헬퍼 함수
	wstring StringToWstring(const std::string& str);

	

public:
	static CAnimationTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;
};
NS_END

