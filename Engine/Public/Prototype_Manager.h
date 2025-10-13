#pragma once
#include "Base.h"

// [Component]
#include "Shader.h"
#include "Texture.h"
#include "Model.h"
#include "VIBuffer_Rect.h"
#include "VIBuffer_Cube.h"
#include "VIBuffer_Point_Instance.h"
#include "Transform.h"
#include "Navigation.h"
#include "Rigidbody.h"
// ==================

NS_BEGIN(Engine)

class CPrototype_Manager final : public CBase
{
private:
	explicit CPrototype_Manager();
	virtual ~CPrototype_Manager() = default;

public:
	HRESULT		Add_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, CBase* pPrototype);
	CBase*		Clone_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, PROTOTYPE eType, void* pArg);
	HRESULT		Clear_Resource(_uint iClearLevelID);

public:
	HRESULT		Initialize(_uint iNumLevel);

private:
	map<const _wstring, CBase*>*		m_Prototypes = { nullptr };
	typedef map<const _wstring, CBase*> PROTOTYPES;

	_uint										m_iNumLevel = {};
	
public:
	static		CPrototype_Manager*	Create(_uint iNumLevel);
	virtual		void							Free() override;
};

NS_END