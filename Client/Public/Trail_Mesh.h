#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Mesh;
class CComputeShader;
NS_END

NS_BEGIN(Client)

class CTrail_Mesh final : public CGameObject
{
public:
	typedef struct tagTrailMeshDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		_wstring strColorTextureTag;
		_wstring strVIBufferTag;

		_int	iShaderPass = 0;
		
		_float	fSweep = 0.f;
		_float	fSweepWitdh = 0.f;

		_int	iDirFlag = 0;

		_float3	vSize = { 1.f, 1.f, 1.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float3 vColor = { 0.f, 0.f, 0.f };
		_float2	vLifeTime = { 0.f, 10.f};
	}TRAILMESH_DESC;

private:
	CTrail_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrail_Mesh(const CTrail_Mesh& Prototype);
	virtual ~CTrail_Mesh() = default;

public:
	virtual HRESULT Initialize_Prototype(const TRAILMESH_DESC* pDesc);
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual void Render();
	
public:
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	void Root_Transform(_fmatrix WorldMatrix);

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CTexture*					m_pColorTextureCom = { nullptr };
	CVIBuffer_Mesh*				m_pVIBufferCom = { nullptr };

	//원형이 들고있을 정보
	TRAILMESH_DESC				m_tDesc = {};

	_int						m_iShaderPass = 0;
		
	//셰이더에 전달할 값 연산용
	_float						m_fSweepSpeed = 0.f;

	//셰이더에 전달할 값
	_float						m_fSweep = 0.f;
	_float						m_fSweepWitdh = 0.f;
	_float						m_fSoft = 0.f;

	_float						m_fColorSpeed = 0.f;
	_float						m_fColorSweep = 0.f;

	_float3						m_vPos = {};
	_float3						m_vColor = {};
	_float2						m_vLifeTime = {};

	_float						m_fTime = 0.f;

	_bool						m_IsRoot = false;
	_float4x4					m_ComBindMatrix = {  };

private:
	HRESULT Ready_Components(TRAILMESH_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CTrail_Mesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const TRAILMESH_DESC* pDesc);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END