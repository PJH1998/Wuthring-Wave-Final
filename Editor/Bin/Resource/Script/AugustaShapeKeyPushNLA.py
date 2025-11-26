import bpy

# Mesh 이름 (필요시 변경)
mesh_name = 'R2T1AogusitaMd10011_LOD0'
mesh = bpy.data.objects.get(mesh_name)

if mesh and mesh.type == 'MESH' and mesh.data.shape_keys:
    shape_keys = mesh.data.shape_keys
    
    # animation_data 생성
    if shape_keys.animation_data is None:
        shape_keys.animation_data_create()
    
    # _Curves 액션 모으기
    curves_actions = [action for action in bpy.data.actions if action.name.endswith('_Curves')]
    
    start_frame = 1  # 모든 스트립 시작 프레임 동일 (프레임 처리 없이 그대로)
    
    for action in curves_actions:
        # 각 액션별 새 NLA 트랙 생성 (파일당 하나처럼 별도 트랙)
        track = shape_keys.animation_data.nla_tracks.new()
        track.name = action.name
        
        # 액션을 NLA 스트립으로 추가 (프레임 그대로)
        strip = track.strips.new(name=action.name, start=start_frame, action=action)
        strip.extrapolation = 'HOLD'
        
        print(f"Added separate track and strip for {action.name} on {mesh_name}'s Shape Keys without frame modifications")
    
    print("All _Curves actions added to separate NLA tracks for Shape Keys.")
else:
    print("Mesh or Shape Keys not found.")