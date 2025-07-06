#version 300 es
precision mediump float;

in mediump vec3 v_color;
in mediump vec2 v_uv;  // UV coordinates từ vertex shader

uniform sampler2D u_texture;  // Texture uniform

layout(location = 0) out mediump vec4 o_color;

void main()
{
	vec4 texColor = texture(u_texture, v_uv);  // Sample texture
	vec3 finalColor = v_color * texColor.rgb;
	o_color = vec4(finalColor, 1.0);
}
