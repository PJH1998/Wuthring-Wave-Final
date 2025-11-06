#pragma once
#include "Base.h"
#include"Client_Enum.h"
#include<variant>

NS_BEGIN(Client)
class CSonoro_Manager final: public CBase
{
private:
	using SonoroObjects = variant<class CMapObject_Sonoro, class CMapObject_NonSonoro>;

private:
	explicit CSonoro_Manager();
	virtual ~CSonoro_Manager() = default;

public:
	void Add_To_Management(OBJECTTYPE eType, void* pObjects);

private:
	unordered_map<OBJECTTYPE, vector<SonoroObjects*>> Objects;

public:
	static CSonoro_Manager* Create();
	virtual void Free()override;
};

NS_END