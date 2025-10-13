#pragma once
#include "Level.h"

#include "Particle.h"
#include "VIBuffer_Point_Instance.h"

NS_BEGIN(Editor)

class CLevel_Effect final : public CLevel
{
public:
	typedef struct ParticleTextureTag {
		const _char* szName = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;

	}PARTICLE_TEXTURE;

private:
	explicit CLevel_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Effect() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

private:
	void Effect_MenuBar();

	void Texture_Loading(const char* TextureName, const _tchar* pFilePath);

	void Particle_Tab();
	CParticle::PARTICLE_DESC Find_Particle(_tchar ParticleTag);

private:
	vector<PARTICLE_TEXTURE>									m_Textures;
	_int														m_iSelectedTexture = -1;
	_bool														m_TexturPopOpend = false;

	_char														m_ParticleTag[MAX_PATH];
	_bool														m_bTagFlag = false;

	map<_tchar*, CParticle::PARTICLE_DESC>						m_tParticleDesc = {};
	map<_tchar*, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC>	m_tVBDesc = {};
	map<_tchar*, class CParticle*>								m_Particles = {};
	map<_tchar*, class CVIBuffer_Point_Instance*>				m_VIBuffers = {};

	_int														m_iSelectedParticle = 0;


public:
	static		CLevel_Effect*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END