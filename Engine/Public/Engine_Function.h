#pragma once
#ifndef Engine_Function_h__
#define Engine_Function_h__

#include "Engine_Typedef.h"

namespace Engine
{
	// 템플릿은 기능의 정해져있으나 자료형은 정해져있지 않은 것
	// 기능을 인스턴스화 하기 위하여 만들어두는 틀

	template<typename T>
	void	Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void	Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete [] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	unsigned long Safe_Release(T& pInstance)
	{
		unsigned long		dwRefCnt = 0;

		if (nullptr != pInstance)
		{
			dwRefCnt = pInstance->Release();

			if (0 == dwRefCnt)
				pInstance = nullptr;
		}

		return dwRefCnt;
	}

	template<typename T>
	unsigned long Safe_AddRef(T& pInstance)
	{
		unsigned long		dwRefCnt = 0;

		if (nullptr != pInstance)
		{
			dwRefCnt = pInstance->AddRef();
		}

		return dwRefCnt;
	}

	inline wstring StringToWString(const string& str)
	{
		int iLength = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		if (0 == iLength)
			return L"";

		wstring wstr(iLength, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], iLength);

		if (!wstr.empty() && wstr.back() == L'\0')
			wstr.pop_back();
		return wstr;
	}

	inline string WStringToString(const wstring& wstr)
	{
		if (wstr.empty())
			return "";

		int iLength = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (0 == iLength)
			return "";

		string str(iLength - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], iLength, nullptr, nullptr);

		return str;
	}

	inline Vec3 LoadVec3(const _float3& vVector) { return Vec3(vVector.x, vVector.y, vVector.z); }
	inline Vec3 LoadVec3(const _fvector& vVector) { return Vec3(vVector.m128_f32[0], vVector.m128_f32[1], vVector.m128_f32[2]); }
	inline _float3 StoreFloat3(const Vec3& vVector) { return _float3(vVector.GetX(), vVector.GetY(), vVector.GetZ()); }
	inline _vector StoreVector3(const Vec3& vVector) { return XMVectorSet(vVector.GetX(), vVector.GetY(), vVector.GetZ(), 0.f); }
	inline Quat LoadQuat(const _float4& vQuat) { return Quat(vQuat.x, vQuat.y, vQuat.z, vQuat.w); }
	inline Quat LoadQuat(const _fvector& vQuat) { return Quat(vQuat.m128_f32[0], vQuat.m128_f32[1], vQuat.m128_f32[2], vQuat.m128_f32[3]); }
	inline _float4 StoreQuat(const Quat& vQuat) { return _float4(vQuat.GetX(), vQuat.GetY(), vQuat.GetZ(), vQuat.GetW()); }
	
	// BoundingBox Local -> World 동기화
	inline void Sync_BoundingBox(BoundingBox* pBox, _fmatrix WorldMatrix) { pBox->Transform(*pBox, WorldMatrix); }
}

#endif // Engine_Function_h__
