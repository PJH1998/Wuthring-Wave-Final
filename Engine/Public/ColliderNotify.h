#pragma once
#include "AnimNotify.h"

NS_BEGIN(Engine)
class ENGINE_DLL CColliderNotify final : public CAnimNotify
{
public:
	explicit CColliderNotify(_float fTrackPosition, const _string& strColliderTag, _bool IsActive);
	virtual void Execute() override;
	virtual json To_Json() const override;
	virtual const _string& Get_NotifyTypeName() const override;

#ifdef _DEBUG
	virtual void ImGui_Print() override;
#endif // _DEBUG

public:
	// ColliderNotify는 특수하게. Set_CallBack을 하나 더만든다.
	//void Set_Callback(const function<void(const _wstring&, _bool)>& callback);
	

public:
	static CColliderNotify* From_Json(const json& colliderJson);
	virtual void Free() override;

private:
	_string m_strColliderTag = {};
	_wstring m_wStrColliderTag = {};
	_bool m_IsActive = {};
};
NS_END
