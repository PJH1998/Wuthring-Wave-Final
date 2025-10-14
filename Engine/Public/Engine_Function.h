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

<<<<<<< Updated upstream
	inline wstring StringToWString(const string& str)
=======
	inline _wstring StringToWString(const _string& str)
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
	inline string WStringToString(const wstring& wstr)
=======
	inline _string WStringToString(const _wstring& wstr)
>>>>>>> Stashed changes
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
}

#endif // Engine_Function_h__
