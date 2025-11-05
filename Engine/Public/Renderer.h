#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CRendererSubResource;
class CTexture;

class CRenderer final : public CBase
{
private:
	explicit CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT				Initialize(_uint iNumThread);
	HRESULT				Add_Render_Object(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	HRESULT				Add_Render_StaticObject(class CStaticObject* pRenderObject);
	HRESULT				Add_Render_ShadowMapObject(class CGameObject* pRenderObject);
	void					Render();
	void					Begin_ScreenEffect(SFX_TYPE eType);
	void					End_ScreenEffect();
	void					Add_Effects(const _wstring& strEffectTag, const vector<ID3DX11Effect*> Effects);
	ID3DX11Effect*		Get_Shader_Effect(const _wstring& strEffectTag, _uint iIndex);

#ifdef _DEBUG
	HRESULT		Add_Render_Debug(class CComponent* pDebugComponent);
	void			Set_LUT_Index(_uint iIndex) { m_iLUT_Index = iIndex; }
	HRESULT		Bind_RawValue(const _char* pConstantName, void* pValue, _uint iLength);
	void			IsSSAO(_bool IsSSao) { m_IsSSAO = IsSSao; }
	void			IsSSAO_Blur(_bool IsBlur) { m_IsSSAO_Blur = IsBlur; }
	void			Setting_SSAO(_float fRadius, _float fMaxDistance);
	void			SetBloomWeight(_int iWeight) { m_iBloomWeight = iWeight; }
	void			SetBloomIntensity(_float fIntensity);
	void			Setting_Fog(_float2 vDepthDistance, _float2 vHeightDistance, _float4 vColor);
	void			SetDof(_float fDepth, _float fRange, _float fScale);
	void			SetMaxEffectIntensity(_float fMaxIntensity) { m_fMaxEffectIntensity = fMaxIntensity; }
	void			SetPBR(_bool IsStylized) { m_IsStylized = IsStylized; }
	void			Set_Metallic(_float fMetallic) { m_fDebugMetallic = fMetallic; }
	void			Set_Roughness(_float fRoughness) { m_fDebugRoughness = fRoughness; }
	void			SetMotionBlur(_float fLimitVelocity, _float fLimitDepth, _float fDistance);
#endif

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*			m_pContext = { nullptr };
	ID3D11DeviceContext**			m_pDeferredContext = { nullptr };
	class CGameInstance*			m_pGameInstance = { nullptr };

	// 비동기 렌더링
	_uint									m_iNumThread = {};
	atomic<_int>						m_iNumEndThread = {};
	vector<ID3D11CommandList*> m_CommandLists;
	map<const _wstring, vector<ID3DX11Effect*>> m_Effects;
	mutex									m_RenderMutex;
	mutex									m_RenderAddMutex;
	condition_variable					m_CV;

	list<class CGameObject*>		m_RenderObjects[ENUM_CLASS(RENDERGROUP::END)];
	vector<class CStaticObject*>	m_StaticObjects[2];
	atomic<_uint>						m_iDoubleBufferIndex = {};
	list<class CGameObject*>		m_ShadowMapObjects;


	class CShader*						m_pShader = { nullptr };
	class CVIBuffer_Rect*				m_pVIBuffer = { nullptr };

	_float4x4								m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	_uint									m_iWinSizeX{}, m_iWinSizeY{};
	_float									m_fWinSizeX{}, m_fWinSizeY{};
	
	CRendererSubResource*			m_pSubResource = { nullptr };
	_uint									m_iLUT_Index = {};
	_int									m_iBloomWeight = { 1 };

	SFX_TYPE							m_eEffectType = { SFX_TYPE::END };
	_bool									m_IsEffectEnd = {};
	_float									m_fEffectIntensity = {};
	_float									m_fMaxEffectIntensity = {};

	recursive_mutex					m_RecursiveMutex;

	_uint									m_iCurTime = {};
	_uint									m_iInterval = {};

#ifdef _DEBUG
	list<class CComponent*>		m_DebugComponents;
	_bool									m_isRenderDebug = { true };
	_bool									m_IsSSAO = { true };
	_bool									m_IsSSAO_Blur = { true };
	_bool									m_IsStylized = { true };
	_float									m_fDebugRoughness = 0.2f;
	_float									m_fDebugMetallic = 0.f;
#endif

private:
	// Viewport Size 
	void						Setting_Viewport(_uint iWinSizeX, _uint iWinSizeY);
	void						Setting_Viewport(ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	// Command List Merge
	void						Merge_CommandList(ID3D11CommandList* pCL, _uint iIndex);

private:
	void						Render_ShadowMap();

	void						Render_Priority();
	void						Render_Shadow();
	void						Render_Outline();
	void						Render_NonBlend();	// 임시
	void						Render_Static();
	void						Render_SSAO();
	void						Render_Decal();
	void						Render_Dynamic();
	void						Render_Light();
	void						Render_Combined();
	void						Render_NonLight();
	void						Render_Emissive();
	void						Render_Bloom();
	void						Render_BloomCombined();
	void						Render_DistortionObject();
	void						Render_Blend();
	void						Render_Distortion();
	void						Render_LUT();
	void						Render_Fog();
	void						Render_ScreenEffect();
	void						Render_UI();
	void						Render_Fade();


	//EFFECT
	void						Update_EffectIntensity();
	void						Render_Blur();
	void						Render_DOF();
	void						Render_MotionBlur();

#ifdef _DEBUG
	void						Render_Debug();
#endif

private:
	void						Render_ObjectList(_uint iRG_Index);

	HRESULT						Ready_RT();
	HRESULT						Ready_MRT();
	HRESULT						Ready_SubResource();
	HRESULT						Ready_RCS();
	HRESULT						Ready_Shadow_DSV();
	HRESULT						Ready_DC();



public:
	static		CRenderer*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumThread);
	virtual		void			Free() override;

};

NS_END