#pragma once
#include "Editor_Define.h"
#include "Base.h"

NS_BEGIN(Editor)

class CModelLoader final : public CBase
{
private:
	explicit CModelLoader();
	virtual ~CModelLoader() = default;

public:
	const aiScene* Get_Scene() { return m_pAIScene; }
public:
	HRESULT		Initialize(MODELTYPE eType, const _char* pModelFilePath);

	HRESULT		Save_Dat_NonAnim(const _char* pFileName);
	HRESULT		Save_Dat_Anim(const _char* pFileName);
	HRESULT		Save_Animation(const _char* pFileName);
	HRESULT		Save_Material(const _char* pFileName);


private:
	const aiScene*		m_pAIScene = { nullptr };
	Assimp::Importer	m_Importer = {};

	MODELTYPE			m_eType = { MODELTYPE::NONANIM };

	

private:
	HRESULT				Save_Texture(json& MaterialData, const aiMaterial* pMaterial, aiTextureType eType);
	HRESULT				Save_Bone(ofstream& OutPut, const aiNode* pNode);

public:
	static		CModelLoader*	Create(MODELTYPE eType, const _char* pModelFilePath);
	virtual		void					Free() override;
};

NS_END