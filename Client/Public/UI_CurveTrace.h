#pragma once
#include "GameObject.h"
#include "VIBuffer_CurveTrace.h"

NS_BEGIN(Client)
class CVIBuffer_CurveTrace;
class CUI_CurveTrace final : public CGameObject
{
public:
	typedef struct tUICurveTraceDesc
	{
		_float3 vStartPos;       // 시작 위치
		_float3 vStartVel;       // 초기 속도
		_float3 vGravity;        // 중력 가속도
		_float  fMaxTime;        // 궤적을 그릴 최대 시간 (0~fMaxTime)
		_uint   iSegmentCount;   // 세그먼트 개수 (기준점 = seg+1)
		_float  fWidth;          // 리본 두께 (월드 단위)

		_float4 vBaseColor;      // 기본 색
		_float4 vHeadColor;      // 시작 쪽 색
		_float4 vTailColor;      // 끝 쪽 색
	}UI_CURVETRACE_DESC;

public:
	explicit CUI_CurveTrace(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_CurveTrace(const CUI_CurveTrace& Prototype);
	virtual ~CUI_CurveTrace() = default;

public:
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override; 



private:
	HRESULT			Ready_Components(void* pArg);
	void			Ready_Presets();

	_float3			EvalProjectilePos(const _float3& vP0, const _float3& vV0, const _float3& vG, _float fT);
	_float3			CalcTangent(_float3* pPts, _uint count, _uint idx);
	_float3			CalcSide(_float3& vTan, _float3& vPos);

private:
	UI_CURVETRACE_DESC	m_tDesc = {};

	CShader*		m_pShaderCom = { nullptr };			// Shader_UI_VtxCurveTrace.hlsl
	CVIBuffer_CurveTrace*	m_pVIBufferCom = { nullptr };		// VIBuffer_CurveTrace.cpp
	_bool			m_isCloned = false;

	_bool			m_isModified = false;

public:
	static CUI_CurveTrace*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};
NS_END