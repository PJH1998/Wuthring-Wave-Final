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

def clean_shape_key_curves_global():
    # 정규식: data_path에서 "key_blocks["KeyName"].value" 형태 파싱
    pattern = re.compile(r'key_blocks\["([^"]+)"\]\.value')
    global_count = 0

    # 1. bpy.data.actions 컬렉션 순회 (블렌더 파일 내의 모든 Action)
    # 💡 모든 애니메이션 데이터를 순회하는 핵심 부분입니다.
    for action in bpy.data.actions:
        print(f"--- Processing Action: {action.name} ---")
        
        fcurves_to_remove = []
        action_modified = False

        for fcurve in action.fcurves:
            match = pattern.search(fcurve.data_path)
            
            # Shape Key 애니메이션 커브가 맞다면
            if match:
                key_name = match.group(1)
                
                # 'data_path'를 사용하여 이것이 Shape Key F-Curve인지 확인하는 것이
                # '_Curves'가 붙은 애니메이션을 찾는 것보다 훨씬 정확합니다.
                # (Shape Key F-Curve의 data_path는 'key_blocks["..."]'로 시작합니다.)
                
                is_target = key_name in TARGET_SHAPE_KEYS
                
                should_remove = False
                if KEEP_ONLY_TARGETS:
                    # Whitelist 모드: 리스트에 없으면 삭제
                    if not is_target:
                        should_remove = True
                        print(f"Queue Delete (Not in Whitelist): {key_name}")
                else:
                    # Blacklist 모드: 리스트에 있으면 삭제
                    if is_target:
                        should_remove = True
                        print(f"Queue Delete (In Blacklist): {key_name}")

                if should_remove:
                    fcurves_to_remove.append(fcurve)
                    action_modified = True

        # 2. 실제 삭제 수행
        current_count = 0
        if action_modified:
            # 리스트를 순회하며 삭제
            for fcurve in fcurves_to_remove:
                action.fcurves.remove(fcurve)
                current_count += 1
            
            global_count += current_count
            print(f"--- Action: {action.name}에서 {current_count}개의 커브 삭제 완료. ---")

    print(f"\n==========================================")
    print(f"--- 최종 완료: 전체 파일에서 총 {global_count}개의 커브를 삭제했습니다. ---")
    print(f"==========================================")
    
    # UI 갱신 (선택된 오브젝트가 없어도 갱신은 필요)
    if bpy.context.view_layer:
        bpy.context.view_layer.update()

# 실행
clean_shape_key_curves_global()