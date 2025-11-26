import bpy
import re

# ==========================================
# 설정 영역
# ==========================================

# 값을 변경할 쉐이프 키 이름 리스트
TARGET_SHAPE_KEYS = [
    "A",
    "O",
    "I",
    "U",
]

# True: 위 리스트에 있는 것만 '변경' (Whitelist)
# False: 위 리스트에 있는 것만 '제외하고 나머지 변경' (Blacklist)
# 보통 특정 키만 줄이고 싶다면 True로 두세요.
APPLY_ONLY_TO_TARGETS = True 

# ------------------------------------------
# 값 조절 설정
# ------------------------------------------

# 변경할 목표 값 (0.5f)
TARGET_FACTOR = 0.5

# 모드 설정 ('SCALE' 또는 'FIXED')
# 'SCALE': 기존 값에 TARGET_FACTOR를 곱합니다. (예: 1.0 -> 0.5, 0.2 -> 0.1) -> 애니메이션 유지됨 (추천)
# 'FIXED': 기존 값을 무시하고 TARGET_FACTOR로 덮어씁니다. (예: 1.0 -> 0.5, 0.2 -> 0.5) -> 표정이 굳음
OPERATION_MODE = 'SCALE'

# ==========================================

def normalize_action_shape_keys():
    # 정규식: "key_blocks["KeyName"].value" 형태 파싱
    pattern = re.compile(r'key_blocks\["([^"]+)"\]\.value')
    
    processed_action_count = 0
    total_modified_curves = 0

    print(f"--- 쉐이프 키 값 조절 시작 (모드: {OPERATION_MODE}, 값: {TARGET_FACTOR}) ---")

    # 블렌더 파일 내의 모든 액션을 순회
    for action in bpy.data.actions:
        
        # 링크된 라이브러리 데이터는 건너뜀
        if action.library:
            continue

        modified_in_action = False

        # 액션 내의 모든 F-Curve 검사
        for fcurve in action.fcurves:
            match = pattern.search(fcurve.data_path)
            
            # 쉐이프키 관련 커브인 경우
            if match:
                key_name = match.group(1)
                is_target = key_name in TARGET_SHAPE_KEYS
                
                should_process = False
                if APPLY_ONLY_TO_TARGETS:
                    # Whitelist: 리스트에 있는 것만 처리
                    if is_target: should_process = True
                else:
                    # Blacklist: 리스트에 없는 것만 처리
                    if not is_target: should_process = True

                # 처리 대상이라면 키프레임 값 수정
                if should_process:
                    modified_in_action = True
                    total_modified_curves += 1
                    
                    for kp in fcurve.keyframe_points:
                        # co[0]은 시간(Frame), co[1]은 값(Value)
                        
                        if OPERATION_MODE == 'SCALE':
                            # 값 곱하기 (핸들도 같이 조절해야 곡선이 안 깨짐)
                            kp.co[1] *= TARGET_FACTOR
                            kp.handle_left[1] *= TARGET_FACTOR
                            kp.handle_right[1] *= TARGET_FACTOR
                            
                        elif OPERATION_MODE == 'FIXED':
                            # 값 고정
                            kp.co[1] = TARGET_FACTOR
                            kp.handle_left[1] = TARGET_FACTOR
                            kp.handle_right[1] = TARGET_FACTOR

                    # 변경 사항 적용을 위해 커브 업데이트
                    fcurve.update()

        if modified_in_action:
            processed_action_count += 1
            # print(f"Updated Action: {action.name}")

    print("--------------------------------------------------")
    print(f"작업 완료.")
    print(f"총 {processed_action_count}개의 액션이 수정되었습니다.")
    print(f"총 {total_modified_curves}개의 F-Curve 키프레임이 재조정되었습니다.")
    
    # UI 갱신
    bpy.context.view_layer.update()

# 실행
normalize_action_shape_keys()