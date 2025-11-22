import bpy
import re

# ==========================================
# 설정 영역
# ==========================================

# 삭제하거나 남길 쉐이프 키 이름 리스트
TARGET_SHAPE_KEYS = [
    "A",
    "O",
    "I",
    "E"
]
# True: 위 리스트에 있는 것만 '남기고' 나머지 삭제 (Whitelist)
# False: 위 리스트에 있는 것만 '삭제' (Blacklist)
KEEP_ONLY_TARGETS = False 

# ==========================================

def clean_all_actions_shape_keys():
    # 정규식: "key_blocks["KeyName"].value" 형태 파싱
    pattern = re.compile(r'key_blocks\["([^"]+)"\]\.value')
    
    total_removed_count = 0
    processed_action_count = 0

    print(f"--- 모든 액션 데이터 정리 시작 (모드: {'남기기' if KEEP_ONLY_TARGETS else '삭제하기'}) ---")

    # 블렌더 파일 내의 모든 액션을 순회
    for action in bpy.data.actions:
        
        # 링크된 라이브러리 데이터(수정 불가)는 건너뜀
        if action.library:
            continue

        fcurves_to_remove = []
        has_target_curves = False

        # 액션 내의 모든 F-Curve 검사
        for fcurve in action.fcurves:
            match = pattern.search(fcurve.data_path)
            
            # 쉐이프키 관련 커브인 경우
            if match:
                has_target_curves = True
                key_name = match.group(1)
                is_target = key_name in TARGET_SHAPE_KEYS
                
                if KEEP_ONLY_TARGETS:
                    # Whitelist: 리스트에 없으면 삭제 대상
                    if not is_target:
                        fcurves_to_remove.append(fcurve)
                else:
                    # Blacklist: 리스트에 있으면 삭제 대상
                    if is_target:
                        fcurves_to_remove.append(fcurve)

        # 삭제 대상이 있다면 실제 삭제 수행
        if fcurves_to_remove:
            print(f"Processing Action: {action.name} | 삭제 예정: {len(fcurves_to_remove)}개")
            for fcurve in fcurves_to_remove:
                action.fcurves.remove(fcurve)
                total_removed_count += 1
            processed_action_count += 1

    print("--------------------------------------------------")
    print(f"작업 완료.")
    print(f"총 {processed_action_count}개의 액션에서 데이터가 수정되었습니다.")
    print(f"총 {total_removed_count}개의 F-Curve가 삭제되었습니다.")
    
    # UI 갱신
    bpy.context.view_layer.update()

# 실행
clean_all_actions_shape_keys()