#pragma once
#include "Base.h"

NS_BEGIN(Editor)
// ImGui Animation Tool
class CAnimationTool final : public CBase
{
private:
	enum class MODE
	{
		SAVE_FBX = 0, 
		LOAD_DAT = 1, 
		EDIT_ANIMATION = 2, // 애니메이션 수정. (Notify)?
		END
	};


private:
	explicit CAnimationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAnimationTool() = default;


#pragma region 기본 함수
public:
	HRESULT	Initialize();
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
	void Render_SaveFBX();
	void Render_LoadDAT();
	void Render_EditDAT();

private:
	// 3 Depth Menu
	


	
	

private:
	MODE m_eMode = { MODE::END };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };



public:
	static CAnimationTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};
NS_END

