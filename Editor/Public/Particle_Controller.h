#pragma once
#include "Base.h"
#include "Particle.h"

NS_BEGIN(Editor)

class CParticle_Controller final : public CBase
{
public:
	typedef struct ParticleTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;

	}PARTICLE_TEXTURE;

private:
	explicit CParticle_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CParticle_Controller() = default;

#pragma region ±âº»
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);
	void Load_AllTextureFromFolder(const _string& strFolderPath);

	void Particle_Tab();
	void UpdateSelected_ParticleFromIndex();
	

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	vector<PARTICLE_TEXTURE>									m_Textures = {};
	_int														m_iSelectedTexture = -1;
	_bool														m_TexturPopOpend = false;

	_char														m_ParticleTag[MAX_PATH];
	_bool														m_bTagFlag = false;

	map<const _wstring, CParticle::PARTICLE_DESC>						m_tParticleDesc = {};
	map<const _wstring, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC>	m_tVBDesc = {};
	map<const _wstring, class CParticle*>								m_Particles = {};

	_int														m_iSelectedParticle = 0;
	_bool														m_bSelectedParticle = false;
	class CParticle*											m_pSelectedParticle = { nullptr };
	CParticle::PARTICLE_DESC*									m_pSelectedParticleDesc = { nullptr };
	CVIBuffer_Point_Instance::POINT_INSTANCE_DESC*				m_pSelectedVBDesc = { nullptr };

public:
	static CParticle_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END