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

		_bool	isUseCustomColor = false;	// 색 임의로 쓸거임? (기본색은 캐릭터별 속성색 프리셋으로 지정)
		_float4 vBaseColor;      // 기본 색
		_float4 vHeadColor;      // 시작 색
		_float4 vTailColor;      // 끝   색
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

public:
	void			Req_Disable() { m_isActivate = false; };

private:
	HRESULT			Ready_Components(void* pArg);
	void			PreAssign_Presets();

	_float3			EvalProjectilePos(const _float3& vPosition, const _float3& vVelocity, const _float3& vAcceleration, _float fTime);
	_float3			CalcTangent(_float3* pPts, _uint count, _uint idx);
	_float3			CalcSide(_float3& vTan, _float3& vPos);

private:
	void			Update_CurveVB();
	void			Update_CurrentColor();

	void			Render_Curve();
	void			Render_Sphere();

private:
	UI_CURVETRACE_DESC	m_tDesc = {};

	_bool			m_isClone = false;
	_bool			m_isModified = false;

private:
	class CGameSystem*	m_pGameSystem = { nullptr };
	
	/// 나중에 CustomUI로 편입시킬 시 수정 필요. 그쪽에 있는 컴포넌트를 사용 해야 함
	CShader*				m_pShaderCom = { nullptr };			// Shader_UI_VtxCurveTrace.hlsl		
	CVIBuffer_CurveTrace*	m_pVIBufferCom = { nullptr };		// VIBuffer_CurveTrace.cpp		

	// 충돌 인디케이터 구(Sphere) 전용 2차 Transform 컴포넌트 및 VIBuffer
	CTransform*				m_pTargetTransformCom = { nullptr };
	CVIBuffer_Sphere*		m_pTargetVIBufferCom = { nullptr };	// VIBuffer_Sphere.cpp		
	_bool					m_isShowTarget = false;				// 구체 켤 거임? 



	array<_float4, 3>	m_arrColorPreset = {};
	array<_float4, 3>	m_arrAdvColorPreset = {};

	array<_float4, 3>	m_arrSelectedColor = {};

public:
	static CUI_CurveTrace*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};
NS_END