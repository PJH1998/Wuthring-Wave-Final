#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CVIBuffer_Point;
NS_END

NS_BEGIN(Editor)
class CEdit_Brush final: public CGameObject
{
private:
	explicit CEdit_Brush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEdit_Brush(const CEdit_Brush& Prototype);
	virtual ~CEdit_Brush() = default;

public:
	virtual		HRESULT			Initialize_Prototype()override;
	virtual		HRESULT			Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;



	void Set_ModelName(const _wstring& pModelName);
private:
	void Bind_Resources();
	void Ready_Components();
	void Foliage();
private:
	CShader* m_pShaderCom = { nullptr };
	CVIBuffer_Point* m_pVIBufferCom = { nullptr };
	_float m_fRange = {};
	_uint m_iNumInstance = {};
	_float3 m_vMousePos = {};
	_float4* m_pPoints= {nullptr};
	_tchar m_ModelName[MAX_PATH] = {};

	_uint m_iMinNum = {};
	_uint m_iMaxNum = {};
	_float m_vMinRotation = {};
	_float m_vMaxRotation = {};
public:
	static CEdit_Brush* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) { return nullptr; }
	virtual void					Free() override;
};

NS_END