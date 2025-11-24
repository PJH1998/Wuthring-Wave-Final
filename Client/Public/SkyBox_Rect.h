#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CVIBuffer_Rect;
class CTexture;
NS_END

NS_BEGIN(Client)

class CSkyBox_Rect final : public CGameObject
{
public:
	typedef struct tagSkyBoxRectDesc {
		_vector				vPosition;
		_vector				vRotateQuaternion;
		_float2				vScale;
		_float3				vColor;
		vector<_wstring>	strTextureTags;
	}SKYBOX_RECT_DESC;

private:
	CSkyBox_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSkyBox_Rect(const CSkyBox_Rect& Prototype);
	virtual ~CSkyBox_Rect() = default;

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
	vector<CTexture*>			m_SkyTextures;

private:
	HRESULT						Ready_Components();

public:
	static CSkyBox_Rect*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END