#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CCustomFont final : public CBase
{
private:
	explicit CCustomFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCustomFont() = default;

public:
	HRESULT						Initialize(const _tchar* pFilePath);
	HRESULT						Render(const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRadian, const _float2& vOrigin, const _float2& vScale);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	SpriteBatch*				m_pBatch = { nullptr };	// Font瑜?洹몃━湲??꾪븳 ?ш컖??踰꾪띁
	SpriteFont*					m_pFont = { nullptr };		// Font

public:
	static		CCustomFont*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath);
	virtual		void				Free() override;
};

NS_END