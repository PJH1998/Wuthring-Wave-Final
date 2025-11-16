#pragma once
#include "StaticObject.h"
#include"Model_Streaming.h"
#include"EditorPch.h"

NS_BEGIN(Editor)
class CEdit_MapObject_Test final : public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
		OBJECTTYPE eObjectType;
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
	}MAP_LOAD;


	typedef struct tagBuffer {
		_char ModelName[MAX_PATH] = {};
	}BUFFER_TEST;
private:
	CEdit_MapObject_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MapObject_Test(const CEdit_MapObject_Test& Prototype);
	virtual ~CEdit_MapObject_Test() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();

	virtual void Set_ImGuiOption();

	HRESULT Ready_Component(void* pArg = nullptr);

private:
	void Export_MaterialData();
	void Child_UpdateMatrix(_fmatrix Matrix, _fvector vParentsPos, _fvector vDeltaTranslation);
	void About_Parent();
	void About_Transform();
	void About_Texture();

private:
	class CShader* m_pShaderCom = { nullptr };
	CModel_Streaming* m_pModelCom = { nullptr };

	_bool m_ExportAllLOD = { true };
	_float4x4 m_ChildLocalMat = {};

	_float4x4 m_DefaultMat = {};
private:

	_char m_ModelName[MAX_PATH];

	_uint m_iShaderPassIndex = {};

	_bool m_IsCustomTexture = { false };
	_bool m_IsLoaded = { false };
	_bool m_MakeJson = { false };
	_bool m_TexMode = { false };

	_uint m_iLevel = {};


public:
	static CEdit_MapObject_Test* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END