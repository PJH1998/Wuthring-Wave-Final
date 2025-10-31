#pragma once

namespace Editor
{
//#pragma region UI
//
//	typedef struct tagUIAnimKeyFrameDesc
//	{
//		unsigned int			iKeyframeIndex = {};	// ?뺣낫媛 ?닿만 ?ㅽ봽?덉엫 ?뺣낫
//
//		unsigned int			iTexIndex = {};
//		float			fAlpha = {};			// 0 ~ 1
//		XMFLOAT3		vPos = {};
//		XMFLOAT3		vRot = {};				// Euler
//		XMFLOAT3		vSca = {};
//
//	} UI_ANIM_KEYFRAME_DESC;
//
//	typedef struct tagUIAnimDesc
//	{
//		class CUSTOM_UI_DESC*	pUIDesc = {};// FilePath, FileName, NumTex (?대뼡 ?띿뒪爾먯슜?몄?瑜??꾪븿)
//
//		// ?ㅽ봽?덉엫, ?ㅽ봽?덉엫蹂??됰젹?뺣낫, 蹂닿컙諛⑸쾿, 湲몄씠 ??.
//		wstring				strAnimName = {};
//		//_uint					iNumKeyFrame = {};
//
//		vector<UI_ANIM_KEYFRAME_DESC*> vecKeyFrames = {};
//
//		unsigned int					iLerpType = {};
//		bool					isLoop = false;
//	} UI_ANIM_DESC;
//
//	typedef struct tagUIInfoDesc
//	{
//		class CUSTOM_UI_DESC*		pUIDesc = {};	// FilePath, FileName, NumTex
//
//		XMFLOAT3					vPos = {};
//		XMFLOAT3					vRot = {};	// Euler
//		XMFLOAT3					vSca = {};
//	} UI_INFO_DESC;
//#pragma endregion

#pragma region EFFECT
	typedef struct tagEffectActorDesc
	{
		class CAnimationActor* pAnimActor = { nullptr }; // Animation 객체 주소
		float fDuration = {}; // 현재선택한 Animation Duration
	}EFFECTACTOR_DESC;
#pragma endregion
}
