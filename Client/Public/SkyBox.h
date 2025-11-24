#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CSkyBox final : public CGameObject
{
public:
	enum class SKYTYPE { DOME, BACKGROUND, FX1, CLOUD, END };
public:
	typedef struct tagSkyBoxDesc {
		vector<_wstring>	strModelTags;
		_uint					iNumModel;
	}SKYBOX_DESC;

private:
	explicit CSkyBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSkyBox(const CSkyBox& Prototype);
	virtual ~CSkyBox() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;
	virtual		void			Render_OutLine() override;
	virtual		void			Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix) override;
	// Pooling Spawn CallBack
	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

private:
	CShader*					m_pShaderCom = { nullptr };
	CModel*						m_pModelCom[ENUM_CLASS(SKYTYPE::END)] = {nullptr};

	_uint						m_iCurrentLevel = {};
	_uint						m_iNumModels = {};

	_float						m_fTimeAcc = {};

private:
	void						Ready_Component(const vector<_wstring>& strModelTags);

public:
	static		CSkyBox*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END