#version 300 es
layout(location = 0) in highp vec3 a_posL;
layout(location = 1) in highp vec3 a_color;
layout(location = 2) in highp vec2 a_uv;  // UV coordinates

out highp vec3 v_color;
out highp vec2 v_uv;  // Truyền UV cho fragment shader

void main()
{
    vec4 posL = vec4(a_posL, 1.0);
    gl_Position = posL;
    v_color = a_color;
    v_uv = a_uv;  // Truyền UV coordinates
}
   