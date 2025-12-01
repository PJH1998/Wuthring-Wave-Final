#pragma once
#include"Base.h"

class CEdit_MapEffectCollector final: public CBase
{
private:
	CEdit_MapEffectCollector();
	virtual ~CEdit_MapEffectCollector() = default;

	HRESULT Initialize();
	void Set_ImGuiOption();

private:
	//필요한 거 거 : 좌표, 트리거 이벤트 번호?(안넣었을 때도 가정)
	//Collaps에 연기는 Collaps에 내가 직접 넣자. Destruction에도 그냥 내가 직접 뼈 위치에 호출하면 될듯?


public:
	static CEdit_MapEffectCollector* Create();
	virtual void Free()override;
};

