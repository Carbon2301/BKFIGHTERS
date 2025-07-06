#version 300 es
layout(location = 0) in highp vec3 a_posL;
layout(location = 1) in highp vec3 a_color;
out highp vec3 v_color;

void main()
{
    vec4 posL = vec4(a_posL, 1.0);
    gl_Position = posL;
    v_color = a_color; 
}
   