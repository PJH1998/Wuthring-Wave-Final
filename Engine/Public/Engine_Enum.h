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
	enum class TEXTURETYPE { DIFFUSE, NORMAL, MASK, EMISSIVE, END };
	enum class MODELTYPE { NONANIM, ANIM, MAP, ECO };
	enum class PROTOTYPE { GAMEOBJECT, COMPONENT };
	enum class RENDERGROUP { PRIORITY, SHADOW, OUTLINE, NONBLEND, STATIC, DYNAMIC, NONLIGHT, EMISSIVE, EFFECT, BLEND, DISTORTION, SFX, UI, FADE,
	#ifdef _DEBUG  
	RD_DEBUG, 
	#endif  
	END };

	enum class EVENT { STATIC, NONSTATIC, END};

	enum class SHADER_DEFFERED { DRAW, RD_DEBUG_CSM, RD_DEBUG_SHAODW_MAP, COMBINED, DIRECTIONAL, POINT, BLOOM, DISTORTION, LUT, FOG, SSAO, DOF, DOF_DEPTH, BLUR, VELOCITY_MAP, MOTION_BLUR, SFX};

	enum class SFX_TYPE { SSAO, BLOOM, BLUR, DOF, RADIAL, MOTION, END};

	enum class SFX_TOGGLE { BLUR = SFX_TYPE::BLUR, DOF = SFX_TYPE::DOF, RADIAL = SFX_TYPE::RADIAL, MOTION = SFX_TYPE::MOTION, END };

	// BroadPhase Layer
	enum class BPLAYER { NONE, NON_MOVE, MOVE, DEBRIS, SENSOR, END };
	// Body Shape
	enum class SHAPE { SPHERE, BOX, CAPSULE, CONVEXHULL, MESH };
	// Collide Timing
	enum class COLLIDE_STATE { ENTER, DURING, REMOVE, END};

	enum class EFFECT_TYPE { PARTICLE, MESH, TRAIL, RECT, DECAL, END };

	// Sequence
	enum class ITEM_TYPE { ACTION, SCENE, SOUND, SFX, ACTOR, EFFECT, END };
}
#endif // Engine_Enum_h__
