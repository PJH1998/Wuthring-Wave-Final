#pragma once
#include "Base.h"
#include"BufferPool.h"

NS_BEGIN(Engine)
class ENGINE_DLL CModel_Manager final : public CBase
{
public:
	enum ModelLoadState { NOTLOADED, LOADING, LOADED };
	
	typedef struct tagModelDataDesc {
		_uint VertexOffset = {};
		_uint IndexOffset = {};

		_uint VertexSize = {};
		_uint IndexSize = {};

		_uint NumIndices = {};
		_float fUnUsedTime = {};

	}SHARED_DATA_DESC;

	typedef struct tagLoadedData {
		_uint iNumIndices;
		vector<VTXMESH> VertexData;
		vector<_uint> IndexData;
	}LOADED_DATA;

	typedef struct tagModelData {
		class CModel_Streaming* pModel = { nullptr };
		_uint iLODIndex;
		vector<LOADED_DATA >LoadData;
	}MODEL_DATA;

	typedef struct tagDeleteData {
		_uint iLODIndex = {};
		_uint VertexOffset = {};
		_uint IndexOffset = {};

		_uint VertexSize = {};
		_uint IndexSize = {};

		_uint iLifeCount = {};
	}DELETE_DATA;

	typedef struct tagPaddingDesc {
		class CModel_Streaming* pModel = { nullptr };
		_uint iLODIndex;
		_uint iDelayFrame;
	}PEDDING_DATA;
private:
	CModel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CModel_Manager() = default;

public:
	HRESULT						Initialize(_uint iMaxLevel);
	void						Update(_float fTimeDelta);


	HRESULT RegisterPrototype(const _char* pFilePath, class CModel_Streaming* pModel);
	//로딩이 완료되면 모델에게 해당 LOD인덱스 모델의 버퍼풀에 해당하는 정보 전달.
	void RequestData(class CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex);
	void SetUp_Data(class CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex);
	void LoadData(class CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex, _fmatrix PreMatrix);
	//m_pModel->Update를 만들어서 각 LOD 단계가 쓰이는지 안쓰이는지 확인하기?

	MODEL_DATA  Acquire_Vector();
	void		Release_Vector(vector<MODEL_DATA>& data);


	void LoadLastLOD();
	void Add_To_RenderTest(_uint iLODIndex, class CStaticObject* pObject);
	void Add_To_RenderTest(vector<class CStaticObject*>* Container);
	void RenderBufferPool(_uint iLODIndex);
	void RenderBufferPool(_uint iThreadIndex, _uint iLODIndex, _uint iStartIndex, _uint iEndIndex, ID3D11DeviceContext* pContext);
	
	void Clear_BufferPool();
	_uint Render_ObjectsNum(_uint iLODIndex);
	void Bind_SharedBuffer(_uint iLODIndex,ID3D11DeviceContext** pDC, _uint iNumThread);
	void Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext* pDC);
	void Destroy_RigidData();

	void Clear_Resource(_uint iLevel);
	void Change_Level(_uint iLevel);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	CBufferPool* m_pBufferPool[4] = { nullptr,nullptr ,nullptr ,nullptr };

	ID3D11Buffer* m_pStagingBuffer = { nullptr };

	//모델을 갖고있기 vs 메쉬를 갖고있기
	unordered_map<_string, class CModel_Streaming*>* m_ModelPrototypes;
	vector<SHARED_DATA_DESC> m_Data;
	vector<MODEL_DATA> m_StagingData;
	vector<MODEL_DATA> m_DataPool;
	vector<PEDDING_DATA> m_DelayedNotice;
	_float4x4 m_PreTransformMatrix = {};
	map<_uint, vector<class CStaticObject*>> m_RenderObjects;
	mutex m_StagingMutex;
	mutex m_RenderMutex;
	mutex m_DataPoolMutex;
	unordered_map<_string, class CModel_Streaming*>::iterator m_iSearchIndex = {};
	//벡터로 데이터 넣는 곳 필요.
	const _uint m_iCheckPerFrame = { 10 };
	_double m_fTotalPlayTime = {};
	vector<DELETE_DATA> m_DeleteList;

	_uint m_iCurrentLoadCnt = {};
	_uint m_iCurrentLevel = {};
	_uint m_iMaxLevel = {};
public:
	static CModel_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iMaxLevel);
	virtual void				Free() override;
};

NS_END