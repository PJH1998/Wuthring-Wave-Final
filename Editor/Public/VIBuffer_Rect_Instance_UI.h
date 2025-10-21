#pragma once
#include "VIBuffer_Instance.h"

NS_BEGIN(Editor)

class CVIBuffer_Rect_Instance_UI final : public CVIBuffer_Instance
{
public:
	typedef struct tagRectSingleInstanceDesc
	{
		// 단일 인스턴스에게 부여할 정보
		_float4 vSInstRight	= {};					// 현재 객체의 Pivot 에 따른 상대좌표
		_float4 vSInstUp	= {};
		_float4 vSInstLook	= {};
		_float4 vSInstTrans	= {};

		_float2 vTexcoordX = {0, 0} ;
		_float2 vTexcoordY = {1, 1} ;

		// hp 등을 위해 클리핑을 위한 비율값도 필요할수도
		_float2 vClipTexcoordX = { 0, 0 }; // based on local space, per single instance
		_float2 vClipTexcoordY = { 1, 1 }; // based on local space, per single instance

	}SINGLE_INST_DESC;

	typedef struct tagRectInstanceUIDesc : public CVIBuffer_Instance::INSTANCE_DESC
	{
		//_float3 vPivot = {};

	}RECT_INSTANCE_UI_DESC;

private:
	explicit CVIBuffer_Rect_Instance_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Rect_Instance_UI(const CVIBuffer_Rect_Instance_UI& Prototype);
	virtual ~CVIBuffer_Rect_Instance_UI() = default;

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC* pDesc) override;
	virtual HRESULT Initialize_Clone(void* pArg) override;
	// virtual HRESULT Bind_Resources() override;
	// virtual HRESULT Render() override;

public:
	void Update_Instances(_float fTimeDelta, vector<SINGLE_INST_DESC>& vecDescs);

private:
	_float3					m_vPivot = {};
	//_float*				m_pSpeeds = {};
	//_bool					m_isLoop = {};

	SINGLE_INST_DESC*		m_pInstanceDesc = {};

public:
	static CVIBuffer_Rect_Instance_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC* pDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
