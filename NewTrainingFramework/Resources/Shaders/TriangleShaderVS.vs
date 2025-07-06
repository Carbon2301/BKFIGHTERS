#version 300 es
layout(location = 0) in highp vec3 a_posL;
layout(location = 1) in highp vec3 a_color;
layout(location = 2) in highp vec2 a_uv;  // UV coordinates

out highp vec3 v_color;
out highp vec2 v_uv;  // Truyền UV cho fragment shader

void main()
{
    // Scale model xuống và center nó
    // Model Woman: Y từ 0.3->1.8, X từ 0->0.2, Z từ -0.16->0.17
    vec3 scaledPos = a_posL;
    
    // Center model: Y center ở 1.05, translate về 0
    scaledPos.y = scaledPos.y - 1.05;
    
    // Scale down để fit trong clip space (-1, 1)
    scaledPos = scaledPos * 0.8;
    
    vec4 posL = vec4(scaledPos, 1.0);
    gl_Position = posL;
    
    // Set point size for GL_POINTS rendering
    gl_PointSize = 3.0;
    
    v_color = a_color;
    v_uv = a_uv;  // Truyền UV coordinates
}
   