#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CVIBuffer_Rect;
class CTexture;
NS_END

NS_BEGIN(Client)

class CLogo_SkyBox final : public CGameObject
{
private:
	enum class SKYBOX { BACK, MID, FRONT, END};

private:
	CLogo_SkyBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLogo_SkyBox(const CLogo_SkyBox& Prototype);
	virtual ~CLogo_SkyBox() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix) override;

private:
	_vector						m_vPosition = {};
	_vector						m_vRotateQuaternion = {};
	_float2						m_vScale = { _float2(1.f, 1.f) };

	CShader*					m_pShader = { nullptr };
	CVIBuffer_Rect*				m_pVIBuffer = { nullptr };
	CTexture*					m_pSkyTextures[ENUM_CLASS(SKYBOX::END)];

#ifdef _DEBUG
	_float3						m_vDebugPosition = {};
	_float3						m_vYawPitchRoll = {};
#endif

private:
	HRESULT						Ready_Components();

public:
	static CLogo_SkyBox*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END