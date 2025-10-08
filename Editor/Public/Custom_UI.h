#pragma once

#include "Editor_Define.h"0
#include "UIObject.h"



NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END


NS_BEGIN(Editor)

class CCustom_UI final : public CUIObject
{
public:
	typedef struct tagCustomUIObjectDesc : public CUIObject::UI_DESC {
		_wstring	strFilePath = {};
		_wstring	strFileName = {};
		_uint		iNumFiles = 1;
	} CUSTOM_UI_DESC;

private:
	explicit				CCustom_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CCustom_UI(const CCustom_UI& Prototype);
	virtual					~CCustom_UI() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)					override;
	virtual void			Priority_Update(_float fTimeDelta)		override;
	virtual void			Update(_float fTimeDelta)				override;
	virtual void			Late_Update(_float fTimeDelta)			override;
	virtual void			Render()								override;

private:
	HRESULT					Ready_Prototypes(void* pArg);
	HRESULT					Ready_Components(void* pArg);

private:
	CShader*				m_pShaderCom				= { nullptr };
	CVIBuffer_Rect*			m_pVIBufferCom				= { nullptr };

	CTexture*				m_pTextureCom				= {};

	// 현재 사용중일 텍스쳐 정보, texcoord 값, 나인섹터 기준점 등의 정보.. 필요할수도 있음

public:
	static CCustom_UI*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

NS_END
