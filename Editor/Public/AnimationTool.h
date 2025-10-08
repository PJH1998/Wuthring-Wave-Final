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
	LEVEL m_eCurLevel = { LEVEL::END };
	MODE m_eMode = { MODE::END };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	class CModelLoader* m_pLoader = { nullptr };

	list<string> m_ModelNames; // 생성된 Model Component들.
	list<string> m_ActorNames; // 생성된 GameObject들.



public:
	static CAnimationTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;
};
NS_END

