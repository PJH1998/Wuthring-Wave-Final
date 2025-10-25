#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class CModel;
class CComputeShader;

class ENGINE_DLL CAnimMachine final : public CComponent
{
public:
	typedef struct tagAnimMacnineDesc
	{
		const _char* pAnimationTag;
	}ANIMMACNINE_DESC;

private:
	explicit CAnimMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAnimMachine(const CAnimMachine& Prototype);
	virtual ~CAnimMachine() = default;

public:
#ifdef _DEBUG
	virtual HRESULT		Initialize_Prototype(/*const _char* AnimMachineDataPath*/) override;
#endif // _DEBUG
	virtual HRESULT		Initialize_Prototype(const _char* AnimMachineDataPath);
	virtual HRESULT		Initialize_Clone(void* pArg) override;

	//void Handle_Input(CModel* pModelCom, _uint* pState, _uint iIndex);
	void Handle_Input(CModel* pModelCom, _uint* pState,_string& strAnimTag);
	void Update(CModel* pModelCom, _uint* pState, _float fTimeDelata);
	void Update(CModel* pModelCom, CComputeShader* pComputeShaderCom,_uint* pState, _float fTimeDelata);
	
	void Reset();

#ifdef _DEBUG
	void Create_AnimStates(const vector<_string>& AnimationNames);
	void Clear_States();
	void Reset_StateData(_string& strAnimName,
						_bool isBlend,
						_bool isRootMotion,
						_float fRootMotionRate,
						_float fTransitTrackPos,
						_float fAnimationSpeed);
	_bool Render_CurrentStateGUI(_string& strCurrentAnim);

	void Save_AnimDatas(json& jsonOutput);
	void Load_AnimDatas(json& jsonInput);
#endif // _DEBUG


private:
	_string m_strCurrentAnimTag;
	//vector<class CAnimState*> m_AnimStates;
	map<_string, class CAnimState*> m_AnimStates;
	_uint m_iCurrentStateIndex{};

public:
#ifdef _DEBUG
	static CAnimMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext /*, const _char* AnimMachineDataPath*/);
#endif // _DEBUG
	static CAnimMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext , const _char* AnimMachineDataPath);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END
