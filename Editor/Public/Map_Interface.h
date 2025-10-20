#pragma once
#include "Interface_Edit.h"
NS_BEGIN(Engine)
class CShader;
class CModel;
class CModel_Instance;
class CTransform;
class CTexture;
NS_END

NS_BEGIN(Editor)
class CMap_Interface final: public CInterface_Edit
{
protected:
	explicit CMap_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMap_Interface() = default;

public:
	virtual		HRESULT			Initialize()override;
	_bool						Set_ShaderPass(CShader* pShader,_uint* ShaderPassIndex);
	_bool						Set_LOD(vector<CModel*>& pModelArray, _uint* iLODIndex);
	void						Set_Transform(CTransform* pTransform);
	_bool						Load_Textures(_uint Origin, vector<_string>* VectorTextures, _string ResearchKeyWord, _string ResearchExt, _bool IsIntoChild, _string TextureFolderPath, _bool IsPng = true, _string SecondKeyWord = "");
	_bool						Display_Textures(CTexture* pTexture, _uint iTextureNum = 0, _float SizeX = 256.f, _float SizeY = 0.f);

	_bool						Initialize_ModelPath(_bool* Test);
	void						Add_MapObject(_uint iLevel, _fmatrix PreTransformMatrix,_fvector vPos = XMVectorSet(0.f, 0.f, 0.f, 0.f));
private:
	_bool m_IsCreateProto = { false };
	vector<_string> m_ModelPaths;
	class CEdit_PreViewModel* m_pPreView = { nullptr };
public:
	static CMap_Interface* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;

};

NS_END