#version 300 es

// Vertex attributes
layout(location = 0) in highp vec3 a_posL;
layout(location = 1) in highp vec3 a_color;
layout(location = 2) in highp vec2 a_uv;

// Uniform MVP matrix  
uniform highp mat4 u_mvpMatrix;

// Output to fragment shader
out highp vec3 v_color;
out highp vec2 v_uv;

void main()
{
    // Apply MVP transformation: MVP = Projection × View × Model
    vec4 position = vec4(a_posL, 1.0);
    gl_Position = u_mvpMatrix * position;
    
    // Set point size for GL_POINTS rendering
    gl_PointSize = 3.0;
    
    // Pass attributes to fragment shader
    v_color = a_color;
    v_uv = a_uv;
}
   