#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class COctoTree final : public CBase
{
private:
	explicit COctoTree();
	virtual ~COctoTree() = default;

public:
	void		SetUp_OctoTree(_float3 vCenter, _float3 vExtent, _uint iDepth);
	void		Add_ToOctoTree(class CGameObject* pObject);

private:
	_uint		m_iDepth = {};

public:
	static		COctoTree*		Create();
	virtual		void				Free() override;
};

NS_END