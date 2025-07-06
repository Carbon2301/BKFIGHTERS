#!/usr/bin/env python3
"""
OBJ to NFG Converter
Converts Wavefront OBJ files to NFG format for the training framework
"""

def parse_obj_file(obj_filename):
    """Parse OBJ file and extract vertices, normals, UVs"""
    vertices = []
    normals = []
    uvs = []
    faces = []
    
    with open(obj_filename, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if not parts:
                continue
                
            if parts[0] == 'v':  # Vertex position
                x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
                vertices.append((x, y, z))
                
            elif parts[0] == 'vn':  # Vertex normal
                x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
                normals.append((x, y, z))
                
            elif parts[0] == 'vt':  # Texture coordinate
                u, v = float(parts[1]), float(parts[2])
                uvs.append((u, v))
                
            elif parts[0] == 'f':  # Face
                face_verts = []
                for vertex_data in parts[1:]:
                    # Format: vertex/texture/normal
                    indices = vertex_data.split('/')
                    v_idx = int(indices[0]) - 1  # OBJ is 1-indexed
                    t_idx = int(indices[1]) - 1 if len(indices) > 1 and indices[1] else 0
                    n_idx = int(indices[2]) - 1 if len(indices) > 2 and indices[2] else 0
                    face_verts.append((v_idx, t_idx, n_idx))
                faces.append(face_verts)
    
    return vertices, normals, uvs, faces

def convert_obj_to_nfg(obj_filename, nfg_filename):
    """Convert OBJ file to NFG format"""
    vertices, normals, uvs, faces = parse_obj_file(obj_filename)
    
    # Create vertex list by expanding faces
    expanded_vertices = []
    
    for face in faces:
        for v_idx, t_idx, n_idx in face:
            pos = vertices[v_idx] if v_idx < len(vertices) else (0, 0, 0)
            norm = normals[n_idx] if n_idx < len(normals) else (0, 1, 0)
            uv = uvs[t_idx] if t_idx < len(uvs) else (0, 0)
            
            # Calculate binormal and tangent (simplified)
            binorm = (0.0, 1.0, 0.0)  # Placeholder
            tangent = (1.0, 0.0, 0.0)  # Placeholder
            
            expanded_vertices.append({
                'pos': pos,
                'norm': norm,
                'binorm': binorm,
                'tangent': tangent,
                'uv': uv
            })
    
    # Write NFG file
    with open(nfg_filename, 'w') as f:
        f.write(f"NrVertices: {len(expanded_vertices)}\n")
        
        for i, vertex in enumerate(expanded_vertices):
            pos = vertex['pos']
            norm = vertex['norm']
            binorm = vertex['binorm']
            tangent = vertex['tangent']
            uv = vertex['uv']
            
            f.write(f"   {i}. pos:[{pos[0]:.6f}, {pos[1]:.6f}, {pos[2]:.6f}]; ")
            f.write(f"norm:[{norm[0]:.6f}, {norm[1]:.6f}, {norm[2]:.6f}]; ")
            f.write(f"binorm:[{binorm[0]:.6f}, {binorm[1]:.6f}, {binorm[2]:.6f}]; ")
            f.write(f"tgt:[{tangent[0]:.6f}, {tangent[1]:.6f}, {tangent[2]:.6f}]; ")
            f.write(f"uv:[{uv[0]:.6f}, {uv[1]:.6f}];\n")
    
    print(f"✅ Converted {obj_filename} → {nfg_filename}")
    print(f"📊 {len(expanded_vertices)} vertices generated")

if __name__ == "__main__":
    # Example usage
    obj_file = "girl_model.obj"  # Input OBJ file
    nfg_file = "Girl.nfg"        # Output NFG file
    
    try:
        convert_obj_to_nfg(obj_file, nfg_file)
    except FileNotFoundError:
        print(f"❌ Error: {obj_file} not found!")
        print("📝 Usage: python ConvertOBJtoNFG.py")
        print("   Make sure you have a .obj file in the same directory") 