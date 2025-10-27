#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CRendererCS;
class CShader;

class CRCS_Manager final : public CBase
{
private:
	typedef map<const _wstring, CRendererCS*> RCS;

private:
	CRCS_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRCS_Manager() = default;

public:
	ID3D11ShaderResourceView* Get_RCS_SRV(const _wstring& strRCSTag);

public:
	HRESULT					Add_RCS(const _wstring& strRCSTag, void* pDesc);
	HRESULT					Add_BufferData(const _wstring& strRCSTag, const _char* pConstantName, void* pData, _uint iLength);
	HRESULT					Add_SRVData(const _wstring& strRCSTag, const _char* pConstantName, ID3D11ShaderResourceView* pSRV);
	HRESULT					Setting_UAV_Data(const _wstring& strRCSTag, const _char* pConstantName);

	HRESULT					Bind_RendererCS(const _wstring& strRCSTag, CShader* pShader, const _char* pConstantName);

	HRESULT					Begin_RCS(const _wstring& strRCSTag);
	void					Clear_RCS(const _wstring& strRCSTag);

#ifdef _DEBUG
	HRESULT                 Debug_Render();
#endif

private:
	ID3D11Device*			m_pDevice = { nullptr }; 
	ID3D11DeviceContext*	m_pContext = { nullptr };
	RCS						m_RCSs;

private:
	CRendererCS*			Find_RCS(const _wstring& strRCSTag);

public:
	static CRCS_Manager*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free();
};

NS_END