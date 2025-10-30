#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class ENGINE_DLL CModel final : public CComponent
{
public:
	enum BUFFER
	{
		BUFFER_KEY_FRAME = 0,
		BUFFER_ANIM_INFO = 1,
		BUFFER_INVERSEBIND_POSE = 2,
		BUFFER_FINAL_BONEMATRIX = 3,
		BUFFER_ANIM_INFOCB = 4, // constant
		BUFFER_STAGING = 5,
		BUFFER_BONE_CHANNEL = 6,
		BUFFER_END
	};


	enum SRV
	{
		SRV_KEY_FRAME = 0,
		SRV_ANIM_INFO = 1,
		SRV_BONE_CHANNEL = 2,
		SRV_INVERSEBIND_POSE = 3,
		SRV_FINAL_BONEMATRIX = 5,
		SRV_END
	};

	enum UAV
	{
		UAV_FINAL_BONEMATRIX = 0,
		UAV_END
	};

private:
	explicit CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CModel(const CModel& Prototype);
	virtual ~CModel() = default;

public:
	_uint								Get_NumMesh() { return m_iNumMeshes; }
	void								Sync_RootNode(class CTransform* pOwnerTransform, class CNavigation* pOwnerNavigation, _float fTimeDelta);
	void								Sync_RootNode(class CTransform* pOwnerTransform, _float fTimeDelta);
	const _float4x4*					Get_BoneMatrixPtr(const _char* pBoneName);
	const vector<_float3>&				Get_VerticesPos(_uint iIndex);
	const vector<_uint>&				Get_Indices(_uint iIndex);
	void Set_TrackPosition(const _string& strAnimName, const _float fTrackPosition);

#ifdef _DEBUG
	const vector<_string>&		Get_AnimationNames() const { return m_AnimationNames; }
	_float*								Get_TrackPositionPtr(const _string& strAnimName);
	_float								Get_Duration(const _string& strAnimName);

	HRESULT Bind_Bone_to_GUI(_int& iBoneIndex, _fmatrix TransformMatrix);
	void Render_Gizmo(_fmatrix TransformMatrix);
#endif

public:
	void								Register_Notify(const _string& strFilePath, const vector<function<void()>>& Functions);
	void								Register_AllNotifies(const _string& strNotifyFolderPath, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback);

public:
	virtual		HRESULT				Initialize_Prototype(MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);
	virtual		HRESULT				Initialize_Clone(void* pArg);
	HRESULT							Render(_uint iMeshIndex);

#ifdef _DEBUG
	_bool								Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif
public:
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType);
	HRESULT							Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	_bool								Play_Animation_CPU(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend = true, _bool isRootMotion = true, _float fRootMotionRate = 0.1f);

	// Compute Shader
	_bool								Play_Animation_GPU(class CComputeShader* pComputeShaderCom, const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isRootMotion = true, _bool isRootMotionRotate = true, _bool isRootMotionTranslate = true, _float fRootMotionRate = 0.1f);

	_bool								Play_Animation(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend = true, _bool isRootMotion = true, _float fRootMotionRate = 0.1f);

	//void								Play_RibAnimation(const _string& strRibAnimationName, _float fTimeDelta);
	void								Play_RibAnimation(const _string& strRibAnimationName, _float fTrackPosition);


	void								Clear_Animation(const _string& strAnimationName, _float fTrackPosition = 0.f);
	

	BoundingBox*							Get_BoundingBox(_uint iNumMesh);
private:
	MODELTYPE							m_eType = { MODELTYPE::NONANIM };

	_uint									m_iNumMeshes = {};
	vector<class CMesh*>				m_Meshes;

	_uint									m_iNumMaterials = {};
	vector<class CMeshMaterial*>	m_Materials;

	_float4x4								m_PreTransformMatrix = {};

	_float4								m_vPreRootRotation = {};
	_float4								m_vPreRootPosition = {};
	_matrix								m_RootMatrix = {};
	_uint									m_iRootBoneIndex = {};
	vector<class CBone*>			m_Bones;

	_uint									m_iNumAnimations = {};
	_string								m_strPreAnimation;
	map<_string, class CAnimation*>		m_Animations;
	map<_string, _uint>					m_AnimationNameToIndex; // Compute Shader

	_bool									m_isBlend = { false };
	_bool									m_isChangeAnimation = { false };

	_float								m_fPreScale = { 0.01f }; // RootMotionRate에 곱해줄 값.

#ifdef _DEBUG
	vector<_string>					m_AnimationNames;
	_uint m_iSelectIndex = { 0 };
#endif
	


#pragma region Compute Shader
private:
	void ApplyComputeResults_ToBones();
	void FetchLocalMatrices_FromCompute(class CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName);

private:
	vector<ID3D11Buffer*> m_Buffers = {};
	vector<ID3D11ShaderResourceView*> m_SRVs = {};
	vector<ID3D11UnorderedAccessView*> m_UAVs = {};

	_bool m_isRibAnimation = { false };

#pragma endregion



private:
	void								Compute_RootAnimation(_float fRootMotionRate, _bool IsRootMotionRotation = true, _bool IsRootMotionTranslate = true);

private:
	HRESULT							Ready_Bone(ifstream& InputFile, _int iParentIndex);
	HRESULT							Ready_Mesh(ifstream& InputFile);
	HRESULT							Ready_Material(const _char* pFilePath);
	HRESULT							Ready_Animation(const _char* pFilePath);

	HRESULT							Ready_Shared_Buffers();
	HRESULT							Ready_Instance_Buffers();


public:
	static		CModel*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);
	virtual		CComponent*		Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END