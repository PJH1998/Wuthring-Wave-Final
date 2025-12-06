import bpy
import json
import os

# ========================================================
# 설정 영역
# ========================================================
# 1. JSON 파일 경로 
JSON_FILE_PATH = JSON_FILE_PATH = r"C:\\Users\\dkznd\\Desktop\\DX11TeamProject\\Wuthring-Wave-Final\\Editor\\Bin\\Resource\\Script\\Burst01.json"

# 2. 생성할 액션 이름
ACTION_NAME = "Burst01_Curves"

# 3. FPS 설정 (사용자 기준: 0.0331 -> 약 30 FPS)
TARGET_FPS = 30

# ========================================================

def load_json_data(filepath):
    if not os.path.exists(filepath):
        print(f"오류: 파일을 찾을 수 없습니다 -> {filepath}")
        return None
    
    with open(filepath, 'r', encoding='utf-8') as f:
        try:
            data = json.load(f)
            return data
        except Exception as e:
            print(f"JSON 로드 중 오류 발생: {e}")
            return None

def find_curve_data(json_data):
    """
    JSON 구조를 순회하며 FloatCurves (커브 데이터)를 찾습니다.
    UE UAsset JSON 덤프 구조에 대응합니다.
    """
    # JSON이 리스트인 경우 (여러 오브젝트가 덤프된 경우)
    if isinstance(json_data, list):
        for item in json_data:
            # Properties 내부 탐색
            props = item.get("Properties", {})
            
            # CompressedCurveData -> FloatCurves 구조 확인
            compressed_data = props.get("CompressedCurveData", {})
            if "FloatCurves" in compressed_data:
                return compressed_data["FloatCurves"]
            
            # 혹은 바로 FloatCurves가 있는 경우
            if "FloatCurves" in props:
                return props["FloatCurves"]
                
    # JSON이 딕셔너리인 경우 (단일 오브젝트)
    elif isinstance(json_data, dict):
        props = json_data.get("Properties", {})
        compressed_data = props.get("CompressedCurveData", {})
        if "FloatCurves" in compressed_data:
            return compressed_data["FloatCurves"]

    return None

def main():
    # 1. 선택된 오브젝트 확인
    obj = bpy.context.object
    if not obj or obj.type != 'MESH':
        print("오류: 메쉬 오브젝트를 선택해주세요.")
        return

    if not obj.data.shape_keys:
        print("오류: 선택한 오브젝트에 쉐이프키(Shape Keys)가 없습니다.")
        return
    
    shape_keys = obj.data.shape_keys
    print(f"대상 오브젝트: {obj.name}")

    # 2. JSON 데이터 로드
    raw_data = load_json_data(JSON_FILE_PATH)
    if not raw_data:
        return

    # 3. 커브 데이터 추출
    float_curves = find_curve_data(raw_data)
    if not float_curves:
        print("오류: JSON 파일 내에서 'FloatCurves' 데이터를 찾을 수 없습니다.")
        print("JSON 구조가 'CompressedCurveData -> FloatCurves' 형태인지 확인해주세요.")
        return

    print(f"총 {len(float_curves)}개의 커브 데이터를 발견했습니다.")

    # 4. 액션(Action) 생성 또는 가져오기
    if not obj.animation_data:
        obj.animation_data_create()
    
    action = bpy.data.actions.get(ACTION_NAME)
    if not action:
        action = bpy.data.actions.new(name=ACTION_NAME)
    
    # 현재 오브젝트에 액션 할당
    obj.animation_data.action = action
    
    # 기존 F-Curve가 있다면 충돌 방지를 위해 정리할 수도 있음 (선택사항)
    # for fc in action.fcurves: action.fcurves.remove(fc)

    # 5. 데이터 파싱 및 키프레임 삽입
    matched_count = 0
    
    for curve_entry in float_curves:
        # JSON 구조: { "CurveName": "Smile", "FloatCurve": { "Keys": [...] } }
        curve_name = curve_entry.get("CurveName")
        
        # 쉐이프키 이름 매칭 확인
        if curve_name not in shape_keys.key_blocks:
            # print(f"스킵: '{curve_name}' 쉐이프키가 블렌더에 없습니다.")
            continue
            
        # 데이터 경로 설정 (쉐이프키 값 조절)
        data_path = f'key_blocks["{curve_name}"].value'
        
        # 해당 쉐이프키의 F-Curve 찾기 또는 생성
        fcurve = action.fcurves.find(data_path)
        if not fcurve:
            fcurve = action.fcurves.new(data_path=data_path)
        
        # 키 데이터 추출
        keys = curve_entry.get("FloatCurve", {}).get("Keys", [])
        
        # 키프레임 입력 (Bulk 처리를 위해 리스트 준비)
        # 기존 키프레임이 있다면 정리하고 새로 넣는 방식 추천
        # 여기서는 간단히 add 로직 사용 (기존 키와 겹치면 덮어쓰지 않고 추가될 수 있으니 주의)
        
        # F-Curve의 기존 포인트 삭제 (깨끗하게 새로 작성)
        fcurve.keyframe_points.clear()
        
        # 키 포인트 추가 준비
        # add() 함수는 갯수만큼 빈 포인트를 생성합니다.
        fcurve.keyframe_points.add(len(keys))
        
        for i, key_data in enumerate(keys):
            time_sec = key_data.get("Time", 0.0)
            value = key_data.get("Value", 0.0)
            
            # === [핵심] 시간 -> 프레임 변환 ===
            frame = round(time_sec * TARGET_FPS)
            
            # 포인트 데이터 설정
            pt = fcurve.keyframe_points[i]
            pt.co = (frame, value) # (x=Frame, y=Value)
            pt.interpolation = 'LINEAR' # 혹은 'BEZIER'

        # F-Curve 업데이트 (핸들 계산 등)
        fcurve.update()
        matched_count += 1

    print("--------------------------------------------------")
    print(f"작업 완료.")
    print(f"블렌더 쉐이프키와 이름이 일치하는 {matched_count}개의 커브를 적용했습니다.")
    print(f"생성된 액션 이름: {ACTION_NAME}")
    print("--------------------------------------------------")

# 실행
if __name__ == "__main__":
    main()