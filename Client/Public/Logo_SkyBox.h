#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CVIBuffer_Rect;
class CModel;
class CTexture;
NS_END

NS_BEGIN(Client)

class CLogo_SkyBox final : public CGameObject
{
private:
	enum class SKYBOX { BACK, FIRST_CLOUD, SEC_CLOUD, EFFECT, END};

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
	CShader*					m_pShader = { nullptr };
	CVIBuffer_Rect*				m_pVIBuffer = { nullptr };
	CTexture*					m_pTexture = { nullptr };
	//CModel*						m_pSkyModel = { nullptr };

	_float4x4					m_CombineMatrix = {};
	_matrix						m_SkyMatrices[ENUM_CLASS(SKYBOX::END)] = {};
	
	_uint						m_iShaderIndex[ENUM_CLASS(SKYBOX::END)] = {};

#ifdef _DEBUG
	_matrix						m_DebugSkyMatrices[ENUM_CLASS(SKYBOX::END)] = {};
	_uint						m_iIndex = 0;
	

	_float3						m_vScale[ENUM_CLASS(SKYBOX::END)] = {};
	_float3						m_vYawPitchRoll[ENUM_CLASS(SKYBOX::END)] = {};
	_float3						m_vPosition[ENUM_CLASS(SKYBOX::END)] = {};
#endif

private:
	HRESULT						Ready_Components();

public:
	static CLogo_SkyBox*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END