import bpy
for obj in bpy.data.objects:
    if obj.animation_data:
        obj.animation_data_clear()  # 모든 NLA 트랙과 액션 클리어

mesh_name = 'R2T1AogusitaMd10011_LOD0'
mesh = bpy.data.objects.get(mesh_name)

if mesh and mesh.type == 'MESH' and mesh.data.shape_keys:
    shape_keys = mesh.data.shape_keys
    
    if shape_keys.animation_data:
        # 모든 NLA 트랙 삭제
        while shape_keys.animation_data.nla_tracks:
            track = shape_keys.animation_data.nla_tracks[0]  # 첫 번째 트랙 선택
            shape_keys.animation_data.nla_tracks.remove(track)
        
        print(f"All NLA tracks removed from {mesh_name}'s Shape Keys.")
        
        # 필요 시 animation_data 자체 클리어 (액션 연결도 해제)
        shape_keys.animation_data_clear()
        print("Animation data cleared.")
    else:
        print("No animation data found on Shape Keys.")
else:
    print("Mesh or Shape Keys not found.")