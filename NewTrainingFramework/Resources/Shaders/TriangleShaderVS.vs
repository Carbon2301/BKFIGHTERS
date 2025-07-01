#version 300 es
layout(location = 0) in highp vec3 a_posL;
void main()
{
vec4 posL = vec4(a_posL, 1.0);
gl_Position = posL;
}
   