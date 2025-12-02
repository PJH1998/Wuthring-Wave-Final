#pragma once
#include"Base.h"

NS_BEGIN(Editor)
class CEdit_MapEffectCollector final: public CBase
{
public:
	typedef struct tagEternalEffect {
		_uint EffectTag = {};
		_float4 vPos = {};
	}ETERNAL_EFFECT;
private:
	CEdit_MapEffectCollector();
	virtual ~CEdit_MapEffectCollector() = default;

	HRESULT Initialize();
	void Set_ImGuiOption();

private:
	//필요한 거 거 : 좌표, 트리거 이벤트 번호?(안넣었을 때도 가정)
	//Collaps에 연기는 Collaps에 내가 직접 넣자. Destruction에도 그냥 내가 직접 뼈 위치에 호출하면 될듯?
	//얘는 영구 이펙트들 하면 될 거 같은데. 영구이펙트들 폴더 분리 요청?
	map<_uint, ETERNAL_EFFECT> m_EffectInfo;
public:
	static CEdit_MapEffectCollector* Create();
	virtual void Free()override;
};

NS_END