#pragma once
#include "GameObject.h"

NS_BEGIN(Client)

class CSFX_Prefab final : public CGameObject
{
public:
	typedef struct tagSFX_PrefabData {
		_float fStartTime = {};
		_wstring strSfxTag = {};
	}SFX_PREFAB_DATA;

	typedef struct tagSFX_PrefabDesc {
		const vector<SFX_PREFAB_DATA>* Children;
	}SFX_PREFAB_DESC;

private:
	CSFX_Prefab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSFX_Prefab(const CSFX_Prefab& Prototype);
	virtual ~CSFX_Prefab() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;
	virtual		void		Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	vector<SFX_PREFAB_DATA> m_Children;  

	_float					m_fCurrentTime = {};

	_uint					m_iNumChildren = {};
	_uint					m_iCurrentChild = {};

public:
	static CSFX_Prefab*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END