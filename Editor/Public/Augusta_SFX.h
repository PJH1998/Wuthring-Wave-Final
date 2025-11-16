#pragma once
#include "Edit_ScreenEffect.h"

NS_BEGIN(Editor)

class CAugusta_SFX final : public CEdit_ScreenEffect
{
private:
	CAugusta_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);


};

NS_END