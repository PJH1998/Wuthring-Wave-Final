#pragma once
#include "Base.h"
#include "Effect_Rect.h"

NS_BEGIN(Editor)

class CDecal_Controller final : public CBase
{
public:
	typedef struct DecalTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;

	}DECAL_TEXTURE;

private:
	explicit CDecal_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDecal_Controller() = default;

#pragma region
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);
	void Load_AllTextureFromFolder(const _string& strFolderPath);

public:
	void Decal_Tab();

	void Decal_Base_Tab(DECAL_DESC& tDecalDesc, _bool& IsCreate);

public:
	void UpdateSelected_DecalFormTag(_wstring DecalTag);
	
	DECAL_DESC* Get_DecalDesc(_wstring& DecalTag);
	void	Set_DecalDesc(_wstring& DecalTag, DECAL_DESC& DecalDesc);

	void Set_DecalTag(const _char* szDecalTag);

	void Remove_Desc(const _wstring& DescTag);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	vector<DECAL_TEXTURE>										m_Textures = {};
	_int														m_iSelectedTexture = -1;
	_bool														m_TexturPopOpend = false;

	_char														m_DecalTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;

	map<const _wstring, DECAL_DESC>								m_tDecalDesc = {};

	_int														m_iSelectedDecal = 0;
	_bool														m_bSelectedDecal = false;
	DECAL_DESC*													m_pSelectedDecalDesc = { nullptr };

	_float														m_fColor[4] = {1.f, 1.f, 1.f, 1.f};		

public:
	static CDecal_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END