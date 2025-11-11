import bpy
import re

# ---------------------- 사용자 설정 ----------------------
# 처리할 액션 이름들 (한 개면 ['MyAction'] 처럼 리스트로, 전부 처리하려면 None)
exclude_prefix = "Ui_"
TARGET_ACTION_NAMES = [
    act.name
    for act in bpy.data.actions
]      # None 으로 두면 현재 선택 오브젝트의 action만 처리
# 무시(초기화+mute)할 뼈 이름 리스트
BONE_NAMES = [""]
BONE_NAMES2 = ["Root"]
# 무엇을 초기화할지 선택
RESET_LOCATION = True
RESET_ROT_EULER = True
RESET_ROT_QUAT = True
RESET_SCALE = False

# 초기화만 하고 mute 안 하려면 False
MUTE_CHANNELS = True

# 채널을 아예 제거하고 싶다면 True (mute 대신 삭제)
REMOVE_CHANNELS_INSTEAD_OF_MUTE = True
# --------------------------------------------------------

def _default_value(data_path: str, array_index: int):
    """data_path와 array_index에 맞는 아이덴티티(기본) 값을 돌려줌."""
    if data_path.endswith("location"):
        return 0.0
    if data_path.endswith("rotation_euler"):
        return 0.0
    if data_path.endswith("rotation_quaternion"):
        # quat: [w, x, y, z] 에서 w=1, 나머지=0
        return 1.0 if array_index == 0 else 0.0
    if data_path.endswith("scale"):
        return 1.0
    # 그 외는 0으로
    return 0.0


def _channel_enabled(data_path: str):
    """사용자 설정(RESET_*)에 따라 이 채널을 초기화 대상에 포함할지 판정."""
    if data_path.endswith("location"):
        return RESET_LOCATION
    if data_path.endswith("rotation_euler"):
        return RESET_ROT_EULER
    if data_path.endswith("rotation_quaternion"):
        return RESET_ROT_QUAT
    if data_path.endswith("scale"):
        return RESET_SCALE
    return False


def _bone_in_datapath(data_path: str, bone_names):
    """
    data_path가 pose.bones["<이름>"].* 형태.
    그 중 bone_names 리스트 중 하나라도 정확히 일치하면 True.
    """
    # data_path 예: pose.bones["UpperArm_L"].rotation_quaternion
    m = re.match(r'^pose\.bones\["(.+?)"\]\.', data_path)
    if not m:
        return False
    bone_name = m.group(1)
    return bone_name in bone_names


def process_action(action: bpy.types.Action, bone_names):
    if not action:
        return 0, 0

    to_remove = []
    touched, muted = 0, 0

    for fcurve in action.fcurves:
        dp = fcurve.data_path

        # pose.bones[...] 이 아닌 건 스킵
        if not dp.startswith('pose.bones'):
            print("Muting")
            continue

        # 대상 뼈 아니면 스킵
        if not _bone_in_datapath(dp, bone_names):
            continue

        # 채널 타입 필터 (사용자 선택)
        if not _channel_enabled(dp):
            continue

        # 1) 값 초기화
        dval = _default_value(dp, fcurve.array_index)
        for kp in fcurve.keyframe_points:
            kp.co[1] = dval
            kp.handle_left[1] = dval
            kp.handle_right[1] = dval
        fcurve.update()
        touched += 1

        # 2) 무시/삭제
        if REMOVE_CHANNELS_INSTEAD_OF_MUTE:
            to_remove.append(fcurve)
        elif MUTE_CHANNELS:
            fcurve.mute = True
            muted += 1

# 필요 시 채널 삭제 
    for fc in to_remove: 
        try: 
            action.fcurves.remove(fc) 
            muted += 1 
        # 삭제도 "무시"의 일종으로 카운트 
        except: 
            pass

    return touched, muted


def main():
    actions = []
    print("사용 가능한 액션들:", list(bpy.data.actions.keys()))
    
    for pb in bpy.context.object.pose.bones:
        for c in pb.constraints:
            c.mute = True
        
    if TARGET_ACTION_NAMES is None:
        obj = bpy.context.object
        if obj and obj.animation_data and obj.animation_data.action:
            actions = [obj.animation_data.action]
        else:
            print("선택된 오브젝트의 활성 액션을 찾을 수 없습니다.")
            return
    else:
        for name in TARGET_ACTION_NAMES:
            act = bpy.data.actions.get(name)
            if act:
                actions.append(act)
            else:
                print(f"[경고] 액션 '{name}'을(를) 찾을 수 없습니다.")

    if not actions:
        print("처리할 액션이 없습니다.")
        return

    total_touched = 0
    total_muted = 0
    for act in actions:
        if exclude_prefix in act.name:
            bones_to_use = BONE_NAMES2  # Root만
        else:
            bones_to_use = BONE_NAMES   # Hair·Skirt 본들
            
        touched, muted = process_action(act, bones_to_use)
        total_touched += touched
        total_muted += muted
        print(f"[{act.name}] 초기화한 채널: {touched}, 무시/삭제 처리된 채널: {muted}")

    print(f"완료: 총 초기화 {total_touched}개, 무시/삭제 {total_muted}개")


if __name__ == "__main__":
    main()
