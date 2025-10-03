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

	enum class KEYSTATE { DOWN, PRESS, UP, END };
	enum class MOUSEKEYSTATE { LB, RB, WB, END };
	enum class MOUSEMOVESTATE { X, Y, WHEEL, END };

	enum class TEXTURETYPE { DIFFUSE, NORMAL, MASK, END };
	enum class MODELTYPE { NONANIM, ANIM };
	enum class PROTOTYPE { GAMEOBJECT, COMPONENT };
	enum class RENDERGROUP { PRIORITY, SHADOW, NONBLEND, NONLIGHT, BLEND, UI, FADE, END };
	enum class EVENT { STATIC, NONSTATIC, END};
}
#endif // Engine_Enum_h__
