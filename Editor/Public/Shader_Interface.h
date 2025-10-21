#pragma once
#include "Interface_Edit.h"

NS_BEGIN(Editor)

class CShader_Interface final : public CInterface_Edit
{
private:
	explicit CShader_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShader_Interface() = default;

public:
	virtual	HRESULT		Initialize();
	void				Update_Shadow();
	
private:
	_float				m_fBias[4] = {};
	_float				m_fMinBias[4] = {};
	_float				m_fSlopeScale = {};
	_uint				m_iLUT_Index = {};
	_float				m_fLUT_Intensity = {};
private:
	void				Set_ShadowBias();
	void				Setting_Bias(const _char* pName, _float* pFloat);
	void				Setting_LUT();

public:
	static CShader_Interface* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END