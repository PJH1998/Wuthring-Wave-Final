#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CVIBuffer_Rect;
class CShader;

class CSFX abstract : public CBase
{
protected:
	typedef struct tagBlurData {
		_float2  vSize;
		_int	 iRadius;
		_float   Padding;
	}BLUR_DATA;

protected:
	CSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CSFX() = default;

public:
	virtual HRESULT			Initialize() { return S_OK; }
	virtual HRESULT			Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) { return S_OK; }

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance*			m_pGameInstance = { nullptr };

public:
	virtual void Free() override;
};

NS_END