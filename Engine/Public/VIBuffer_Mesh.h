#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Mesh final : public CVIBuffer
{
private:
	explicit CVIBuffer_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Mesh(const CVIBuffer_Mesh& Prototype);
	virtual ~CVIBuffer_Mesh() = default;

public:
	virtual HRESULT Initialize_Prototype(_fmatrix PreTransformMatrix, const _char* pFilePath);
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	static CVIBuffer_Mesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, _fmatrix PreTransformMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
