#pragma once
#include"StaticObject.h"


NS_BEGIN(Engine)
class CModel;
class CShader;
class CRigidbody;
class CTexture;
NS_END


NS_BEGIN(Editor)
//class CEdit_MapObject : public CStaticObject
	class CEdit_MapObject : public CGameObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
	}MAP_LOAD;

	typedef struct tagMapSave
	{
		_uint m_iNameLength;
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex;
		_float4x4 WorldMatrix;
	}OBJECT_SAVE;

protected:
	CEdit_MapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MapObject(const CEdit_MapObject& Prototype);
	virtual ~CEdit_MapObject() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	//virtual		void			Render(_uint iLOD = 0);
	virtual		void			Render();
	virtual		void			Render_Shadow();

	void Set_ImGuiOption();

	HRESULT Ready_Component(void* pArg = nullptr);

	void Bind_Resources();

	_char* Get_ModelName() { return m_ModelName; }

	void Add_Child(CEdit_MapObject* pObject);
	void Quit_Child(CEdit_MapObject* pObject);
	void Make_ChildLocalMatrix(_fmatrix ParentMatrix);
	void Set_ShaderPass(_uint i) { m_iShaderPassIndex = i; }

private:
	void Export_MaterialData();
	void Child_UpdateMatrix(_fmatrix Matrix, _fvector vParentsPos, _fvector vDeltaTranslation);
	void About_Parent();
	void About_Transform();
	void About_Texture();
private:
	CModel* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };

	vector<CModel*> m_pModelComArray;
	CRigidbody* m_pRigidbodyCom = { nullptr };
	CEdit_MapObject* m_pParent = { nullptr };
	list<CEdit_MapObject*> m_ChildObjects;

	CEdit_MapObject* m_pPickedChild = { nullptr };
	_bool m_IsSetParent = { false };
	_bool m_IsParent = { false };
	vector<CTexture*> m_pDiffuseTextureCom;
	vector<CTexture*> m_pNormalTextureCom;



	_float4x4 m_ChildLocalMat = {};
private:
#ifdef _DEBUG
	_char m_ModelName[MAX_PATH];
#endif

	_uint m_iShaderPassIndex = {};
	_float3 m_vScale = {};
	_float3 m_vRotation = {};
	_float3 m_vTranslation = {};

	_float3 m_vNewScale = {};
	_float3 m_vNewRotation = {};
	_float3 m_vNewTranslation = {};

	_bool m_IsCustomTexture = { false };
	_bool m_IsLoaded = { false };
	_bool m_MakeJson = { false };
	_bool m_TexMode = { false };
	
	//?대뜑 援ъ“?濡? ?ㅻ툕?앺듃?먯꽌 踰꾪듉 ?꾨Ⅴ硫??대뜑 ?꾩튂 ?↔퀬 洹??꾩튂瑜??쎄쾶? 
	vector<_string> m_DiffuseTextureName;
	vector<_string> m_NormalTextureName;

	vector<_string> m_SelectedDiffuseTextureName;
	vector<_string> m_SelectedNormalTextureName;

	_string m_SelectedDiffuse;
	_string m_SelectedNormal;
	vector<_string >m_SelectedDiffuseName;
	vector<_string >m_SelectedNormalName;
	_string m_iSelectedMeshName;
	_uint m_iSelectedMesh={};

	_uint* m_iSelectedDiffuseIndex;
	_uint* m_iSelectedNormalIndex;

	_uint m_iNumObject = {};
private:
	static _uint g_iNumObjects;
public:
	static CEdit_MapObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END