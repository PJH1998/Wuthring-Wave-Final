import bpy

# ==========================================
# [설정] 본인의 오브젝트 이름으로 정확히 수정하세요!
armature_name = "R2T1CalbrenaMd10011_LOD0_Skeleton"
mesh_name = "R2T1CalbrenaMd10011_LOD0"
# ==========================================

def setup_nla_separate_tracks():
    arm_obj = bpy.data.objects.get(armature_name)
    mesh_obj = bpy.data.objects.get(mesh_name)

    if not arm_obj or not mesh_obj:
        print("❌ 오브젝트를 찾을 수 없습니다. 이름을 확인해주세요.")
        return

    # 1. 애니메이션 데이터 초기화 (기존 트랙 싹 지우기)
    if not arm_obj.animation_data: arm_obj.animation_data_create()
    if not mesh_obj.animation_data: mesh_obj.animation_data_create()
    
    # 기존 트랙 제거 (오류 방지)
    for track in arm_obj.animation_data.nla_tracks:
        arm_obj.animation_data.nla_tracks.remove(track)
    for track in mesh_obj.animation_data.nla_tracks:
        mesh_obj.animation_data.nla_tracks.remove(track)
        
    # 현재 활성화된 액션 링크 해제 (이게 남아있으면 겹쳐 보임)
    arm_obj.animation_data.action = None
    mesh_obj.animation_data.action = None

    print(f"🚀 NLA 트랙 생성 시작 (Action별 개별 트랙)...")

    # 2. 모든 액션 순회
    for action in bpy.data.actions:
        # 쉐이프키 액션이나 불필요한 액션 건너뛰기
        if action.name.startswith("Key|") or action.name.startswith("Shader"):
            continue
            
        print(f"   Processing: {action.name}")

        # (A) 몸통용 새 트랙 생성 (이름을 액션 이름으로)
        # 중요: 액션마다 새 트랙을 만드므로 '공간 부족' 에러가 안 남
        arm_track = arm_obj.animation_data.nla_tracks.new()
        arm_track.name = action.name 
        
        # (B) 얼굴용 새 트랙 생성
        mesh_track = mesh_obj.animation_data.nla_tracks.new()
        mesh_track.name = action.name

        # (C) 스트립 추가
        try:
            # 몸통 스트립
            strip = arm_track.strips.new(action.name, int(action.frame_range[0]), action)
            strip.name = action.name
            
            # 짝이 맞는 얼굴 액션 찾기
            shape_name = "Key|" + action.name 
            shape_action = bpy.data.actions.get(shape_name)
            
            if shape_action:
                m_strip = mesh_track.strips.new(action.name, int(action.frame_range[0]), shape_action)
                m_strip.name = action.name
                print(f"      ✅ Linked Pair: {action.name} + {shape_name}")
            else:
                print(f"      ⚠️ No ShapeKey for {action.name}")
                
        except Exception as e:
            print(f"      ❌ Error adding strip: {e}")

    print("🎉 정리 완료! NLA 창을 확인해보세요.")

setup_nla_separate_tracks()