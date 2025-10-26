#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CModel;
class CAnimTransition;
class CAnimMachine;
class CComputeShader;

class ENGINE_DLL CAnimState : public CBase
{
public:
	typedef struct tagAnimStateDesc
	{
		//_float* pTrackPosition;
		_bool isBlend;
		_bool isRootMotion;
		_bool isLoop;
		_float fRootMotionRate;
		_float fTransitTrackPos;
		_float fAnimationSpeed;
	}ANIMSTATE_DESC;
protected:
	explicit CAnimState() = default;
	virtual ~CAnimState() = default;

public:
#ifdef _DEBUG
	virtual HRESULT		Initialize(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc);
	void				Set_Data(ANIMSTATE_DESC& StateDesc);
	const ANIMSTATE_DESC Get_StateData() { return m_StateData; }
#endif

	virtual HRESULT		Initialize(json& jsonParser, json& jsonTransitions);
	virtual void		Enter(CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag);
	virtual void		Update(CAnimMachine* pAnimMachine, CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag, _float fTrackPosition/*, ANIMSTATE_DESC& StateData*/);
	virtual void		Exit(CModel* pModelCom, _uint* pOwnerState);
	virtual _bool		Play_Animation(CModel* pModelCom, _float fTimeDelta);
	virtual _bool		Play_Animation_GPU(CModel* pModelCom, CComputeShader* pComputeShaderCom, _float fTimeDelta);
	virtual void		Feedback(_bool isAnimationFinished, _uint* pOwnerState, CAnimMachine* pAnimMachineCom, CModel* pModelCom);
	//virtual void Reset() PURE;

#ifdef _DEBUG
	void				Render_GUI();
#endif

private:
	_string m_strAnimationTag;

	_float m_fCurrentTrackPositon{};

	//애니메이션 데이터 원본
	ANIMSTATE_DESC m_StateData{};

	//상황에 따른 변동 변수
	_bool	m_isBlend{};
	_bool	m_isRootMotion{};
	_bool	m_isLoop{};
	_float	m_fRootMotionRate{};
	_float	m_fTransitTrackPos{};
	_float	m_fAnimationSpeed{};

	vector<CAnimTransition*> m_Transitions;

public:
#ifdef _DEBUG
	static CAnimState* Create(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc);
#endif
	static CAnimState* Create(json& jsonState, json& jsonTransitions);
	//virtual CAnimState* Clone() PURE;
	virtual void Free() override;
};
NS_END
