#pragma once
#include"GameObject.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
class CTexture;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CPotal: public CGameObject
{
public:
	typedef struct tagPotalDesc {
		_uint iLevel = {};
		_float3 vExtent = _float3(2.f, 2.f, 2.f);
		_float4 vPos = {};
	}POTAL_DESC;
private:
	CPotal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPotal(const CPotal& Prototype);
	virtual ~CPotal() = default;


public:
	virtual		HRESULT			Initialize_Prototype();
	virtual		HRESULT			Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	void						PotalActive(_bool B);
private:
	void						Change_Level();
	void						Ready_Components(void* pArg);

private:
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };
	CTexture*			m_pDiffuseCom = { nullptr };
	CTexture*			m_pFirstMaskCom = { nullptr };
	CTexture*			m_pSecondMaskCom = { nullptr };
	class CGameSystem*  m_pGameSystem = { nullptr };
	CRigidbody*			m_pRigidbodyCom = { nullptr };
	_wstring			m_szText;
	_float				m_fTotalTime = {};
	_float4				m_vColor = {};
public:
	static CPotal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END