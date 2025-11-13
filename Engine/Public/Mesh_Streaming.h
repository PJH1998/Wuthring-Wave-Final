#pragma once
#include "Component.h"
#include"Model_Manager.h"

NS_BEGIN(Engine)
class CMesh_Streaming final : public CComponent
{
private:
	explicit CMesh_Streaming(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMesh_Streaming(const CMesh_Streaming& Prototype);
	virtual ~CMesh_Streaming() = default;

public:
	virtual		HRESULT				Initialize_Prototype(_uint iNumMeshes);
	virtual		HRESULT				Initialize_Clone(void* pArg);

	atomic<LOADSTATE>&  IsLoaded() { return m_LoadState; }
	HRESULT Render(_uint iNumMeshIndex);

public:
	vector<CModel_Manager::SHARED_DATA_DESC>* Get_MeshDesc() { return m_Desc; }

private:
	//애매함.
	vector<CModel_Manager::SHARED_DATA_DESC>* m_Desc;
	atomic<LOADSTATE> m_LoadState = { LOADSTATE::NOTLOADED };
	_uint m_iNumMeshes = {};
	ID3D11Buffer* m_pSharedVB = { nullptr };
	ID3D11Buffer* m_pSharedIB = { nullptr };
public:
	static CMesh_Streaming* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumMeshes);
	virtual void Free()override;

	// CComponent을(를) 통해 상속됨
	CComponent* Clone(void* pArg) override;
};

NS_END