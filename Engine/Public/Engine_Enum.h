#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{
	enum class D3DTS { VIEW, PROJ, END };
	enum class STATE { RIGHT, UP, LOOK, POSITION };
	enum class STATIC { STATIC, NONE };
	enum class WINMODE { FULL, WIN };
	enum class POINTS { A, B, C, END};
	enum class LINE { AB, BC, CA, END };

	// Input
	enum class KEYSTATE { DOWN, PRESS, UP, END };
	enum class MOUSEKEYSTATE { LB, RB, WB, END };
	enum class MOUSEMOVESTATE { X, Y, WHEEL, END };

	// Render
	enum class TEXTURETYPE { DIFFUSE, NORMAL, MASK, END };
	enum class MODELTYPE { NONANIM, ANIM, MAP };
	enum class PROTOTYPE { GAMEOBJECT, COMPONENT };
	enum class RENDERGROUP { PRIORITY, SHADOW, OUTLINE, NONBLEND, NONLIGHT, EMISSIVE, BLEND, DISTORTION, UI, FADE,  
	#ifdef _DEBUG  
	RD_DEBUG, 
	#endif  
	END };

	enum class EVENT { STATIC, NONSTATIC, END};

	enum class SHADER_DEFFERED { RD_DEBUG, RD_DEBUG_CSM, COMBINED, DIRECTIONAL, POINT, GAUSSIAN_BLUR_X, GAUSSIAN_BLUR_Y, SSAO_BLUR_X, SSAO_BLUR_Y, BLUR_COMBINED, DISTORTION, LUT, SSAO};
	// BroadPhase Layer
	enum class BPLAYER { NON_MOVE, MOVE, DEBRIS, SENSOR, END };
	// Body Shape
	enum class SHAPE { SPHERE, BOX, CAPSULE, CONVEXHULL, MESH };
	// Collide Timing
	enum class COLLIDE_STATE { ENTER, DURING, REMOVE, END};

	enum class EFFECT_TYPE { PARTICLE, MESH, END };
}
#endif // Engine_Enum_h__
