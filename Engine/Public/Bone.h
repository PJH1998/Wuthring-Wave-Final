#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CBone final : public CBase
{
private:
	explicit CBone();
	virtual ~CBone() = default;

public:
	const _char*				Get_Name() { return m_szName; }
	const _float4x4*		Get_CombinedTransformationMatrix() { return &m_CombinedTransformationMatrix; }
	const _float4x4*		Get_TransformationMatrix() { return &m_TransformationMatrix; }
	const _int				Get_ParentBoneIndex() { return m_iParentBoneIndex; }
	void						Set_TransformationMatrix(const _fmatrix& Matrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, Matrix);
	}
public:
	HRESULT					Initialize(const _char* pBoneName, const _fmatrix& TransformationMatrix, _int iParentBoneIndex);
	void						Update_CombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const vector<CBone*>& Bones);
	void						Update_CombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const _float4x4* pBoneMatrix, const vector<CBone*>& Bones);

private:
	_float4x4					m_TransformationMatrix = {};
	_float4x4					m_CombinedTransformationMatrix = {};

	_int						m_iParentBoneIndex = {};
	_char						m_szName[MAX_PATH] = {};

public:
	static		CBone*		Create(const _char* pBoneName, const _fmatrix& TransformationMatrix, _int iParentBoneIndex);
	CBone*	Clone();
	virtual		void			Free() override;
};

NS_END