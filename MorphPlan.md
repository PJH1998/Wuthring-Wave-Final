## Morph Target 업데이트 개선 계획

### 1. 현재 구조 정리
- **Morph 애니메이션 갱신 위치**
  - `CModel::Play_Animation_GPU` / `CModel::Play_FlyAnimation_GPU` 에서 `Update_MorphAnimation` 호출.
  - 구현부: `CModel::Update_MorphAnimation` (`Engine/Private/Model.cpp` 850줄 부근).
- **현재 동작**
  - `pAnimation->Update_MorphWeights(fTimeDelta, m_ShapeKeyWeights);`
  - Morph Weight를 GPU 버퍼 `BUFFER_MORPH_WEIGHT`에 복사.
  - 이후 **모든** `m_Meshes`에 대해:
    - `pMesh->Compute_Morph(pMorphComputeShaderCom, m_SRVs[SRV_MORPH_WEIGHT]);`
  - 메쉬가 Morph Target(ShapeKey)이 없는 경우도 동일하게 `Compute_Morph`를 타게 되어, 불필요한 Dispatch가 발생.

### 2. 목표
- **Model 레벨에서 Morph Target(Shape Key)을 가진 메쉬만 Compute Morph를 수행**하도록 변경.
- Morph 관련 버퍼 초기화(`Ready_Mesh_MorphBuffers`, `Ready_SharedBuffers_ForMorph`)도 **Morph 타겟이 있는 메쉬에만** 수행되도록 정리.

### 3. 설계 방향
1. **메쉬 단위로 "Morph 타겟 존재 여부" 질의 함수 추가**
   - 대상: `Engine/Public/Mesh.h`, `Engine/Private/Mesh.cpp`
   - 신규 함수 예:
     - `bool HasMorphTargets() const { return !m_ShapeKeys.empty() && m_iNumAnimMeshes > 0; }`
   - 기준:
     - `m_ShapeKeys` 비어 있지 않음.
     - 또는 `m_iNumAnimMeshes > 0` (파일에서 읽어온 Morph Mesh 개수).

2. **Compute 단계에서 필터링**
   - 대상: `CModel::Update_MorphAnimation` (`Engine/Private/Model.cpp`)
   - 변경 전:
     - `for (auto& pMesh : m_Meshes) { pMesh->Compute_Morph(...); }`
   - 변경 후:
     - `for (auto& pMesh : m_Meshes) { if (!pMesh->HasMorphTargets()) continue; pMesh->Compute_Morph(...); }`
   - 효과:
     - Morph 타겟이 없는 메쉬는 Compute Shader Dispatch에서 제외되어, 불필요한 GPU 연산 제거.

3. **Buffer 생성 단계에서도 Morph 있는 메쉬만 처리**
   - 대상:
     - `CModel::Ready_Mesh_MorphBuffers` (`Engine/Private/Model.cpp`)
     - `CMesh::Ready_SharedBuffers_ForMorph` (`Engine/Private/Mesh.cpp`)
   - 계획:
     - `Ready_Mesh_MorphBuffers`에서:
       - `for (auto& pMesh : m_Meshes)` 반복 시 `if (!pMesh->HasMorphTargets()) continue;` 조건 추가.
     - `Ready_SharedBuffers_ForMorph`는 **Morph 타겟이 없는 경우에도 실패(E_FAIL)를 반환하지 않고 단순 통과(S_OK)** 하도록 보강하거나,
       - 상위(Model)에서 애초에 HasMorphTargets 체크로 호출 자체를 막는 방식 사용.

4. **안전 장치: CMesh::Compute_Morph 내부 가드 추가**
   - 대상: `CMesh::Compute_Morph` (`Engine/Private/Mesh.cpp`)
   - 추가 조건:
     - `if (m_ShapeKeys.empty() || m_iNumAnimMeshes == 0) return;`
   - 이미 `MODELTYPE::CHARACTER`, `m_IsSharedReady`, `m_IsInstanceReady` 체크가 있지만,
     - ShapeKey 없는 경우를 방어적으로 한 번 더 막아서 크래시 및 잘못된 버퍼 접근 가능성을 줄임.

### 4. 실제 작업 순서
1. `CMesh`에 `HasMorphTargets()` (혹은 유사한 헬퍼 함수) 선언 및 정의 추가.
2. `CModel::Update_MorphAnimation`에서 메쉬 순회 시 `HasMorphTargets()` 조건으로 필터링.
3. `CModel::Ready_Mesh_MorphBuffers`에서 역시 `HasMorphTargets()` 조건으로 Morph 버퍼 생성 대상 메쉬를 제한.
4. 필요 시 `CMesh::Ready_SharedBuffers_ForMorph`와 `CMesh::Compute_Morph` 내부에 ShapeKey 유무를 체크하는 early-return 가드 추가.
5. Morph 타겟이 없는 캐릭터/에코/기타 모델이 있을 때도 초기화 및 재생이 정상 동작하는지 테스트:
   - Morph 없는 일반 캐릭터 메시 렌더링/애니메이션.
   - Facial Morph가 있는 캐릭터의 얼굴 메시만 Morph가 갱신되는지 확인.

### 5. 예상 영향 범위
- **수정 파일**
  - `Engine/Public/Mesh.h`
  - `Engine/Private/Mesh.cpp`
  - `Engine/Private/Model.cpp`
- **런타임 영향**
  - Morph 타겟이 없는 메시에서는:
    - Morph 관련 버퍼 미생성 혹은 생성 후 미사용.
    - Compute Shader Dispatch 호출 제거 → 약간의 GPU 성능 향상.
  - Morph 타겟이 있는 메시에서는:
    - 기존과 동일하게 Weight 기반 Morph 계산 및 `g_MorphedVertices` 바인딩.

