
#ifndef Editor_Struct_h__
#define Editor_Struct_h__

namespace Editor
{
#pragma region UI

	typedef struct tagUIAnimKeyFrameDesc
	{
		unsigned int			iKeyframeIndex = {};	// 정보가 담길 키프레임 정보

		unsigned int			iTexIndex = {};
		float			fAlpha = {};			// 0 ~ 1
		XMFLOAT3		vPos = {};
		XMFLOAT3		vRot = {};				// Euler
		XMFLOAT3		vSca = {};

	} UI_ANIM_KEYFRAME_DESC;

	typedef struct tagUIAnimDesc
	{
		class CUSTOM_UI_DESC*	pUIDesc = {};// FilePath, FileName, NumTex (어떤 텍스쳐용인지를 위함)

		// 키프레임, 키프레임별 행렬정보, 보간방법, 길이 등..
		wstring				strAnimName = {};
		//_uint					iNumKeyFrame = {};

		vector<UI_ANIM_KEYFRAME_DESC*> vecKeyFrames = {};

		unsigned int					iLerpType = {};
		bool					isLoop = false;
	} UI_ANIM_DESC;

	typedef struct tagUIInfoDesc
	{
		class CUSTOM_UI_DESC*		pUIDesc = {};	// FilePath, FileName, NumTex

		XMFLOAT3					vPos = {};
		XMFLOAT3					vRot = {};	// Euler
		XMFLOAT3					vSca = {};
	} UI_INFO_DESC;

#pragma endregion





}

#endif // Engine_Struct_h__
