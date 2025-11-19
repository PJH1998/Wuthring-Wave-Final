import bpy
import re

# 삭제하고 싶은 쉐이프 키 이름
TARGET_SHAPE_KEYS = [
    "A",
    "I",
    "O"
]

# True면 위 리스트에 있는 것만 '남기고' 나머지 다 삭제 (Whitelist 모드)
# False면 위 리스트에 있는 것만 '삭제' (Blacklist 모드)
KEEP_ONLY_TARGETS = False 
# ==========================================

def clean_shape_key_curves():
    obj = bpy.context.object
    
    # 1. 안전 장치: 오브젝트 선택 확인
    if not obj or obj.type != 'MESH':
        print("Error: 메쉬 오브젝트를 선택해주세요.")
        return

    # 2. 애니메이션 데이터 확인
    # 쉐이프키 애니메이션은 보통 Object의 Action에 섞여있거나, 
    # ShapeKeys 데이터 블록 자체의 Action에 있을 수 있습니다.
    # 여기서는 현재 활성화된(눈에 보이는) Action을 타겟으로 합니다.
    action = None
    if obj.animation_data and obj.animation_data.action:
        action = obj.animation_data.action
    elif obj.data.shape_keys.animation_data and obj.data.shape_keys.animation_data.action:
        action = obj.data.shape_keys.animation_data.action
        
    if not action:
        print("Error: 선택된 오브젝트에 활성화된 Action(애니메이션)이 없습니다.")
        return

    print(f"--- Processing Action: {action.name} ---")
    
    # 삭제할 F-Curve들을 담을 리스트
    fcurves_to_remove = []
    
    # 정규식: data_path에서 "key_blocks["KeyName"].value" 형태 파싱
    # 예: key_blocks["F_Smile"].value -> "F_Smile" 추출
    pattern = re.compile(r'key_blocks\["([^"]+)"\]\.value')

    for fcurve in action.fcurves:
        match = pattern.search(fcurve.data_path)
        
        # Shape Key 애니메이션 커브가 맞다면
        if match:
            key_name = match.group(1)
            
            is_target = key_name in TARGET_SHAPE_KEYS
            
            if KEEP_ONLY_TARGETS:
                # Whitelist 모드: 리스트에 없으면 삭제
                if not is_target:
                    fcurves_to_remove.append(fcurve)
                    print(f"Queue Delete (Not in Whitelist): {key_name}")
            else:
                # Blacklist 모드: 리스트에 있으면 삭제
                if is_target:
                    fcurves_to_remove.append(fcurve)
                    print(f"Queue Delete (In Blacklist): {key_name}")

    # 3. 실제 삭제 수행
    # 리스트를 순회하며 삭제 (Loop 도중 삭제로 인한 인덱스 오류 방지)
    count = 0
    for fcurve in fcurves_to_remove:
        action.fcurves.remove(fcurve)
        count += 1
        
    print(f"--- 완료: 총 {count}개의 커브를 삭제했습니다. ---")
    
    # UI 갱신
    bpy.context.view_layer.update()

# 실행
clean_shape_key_curves()