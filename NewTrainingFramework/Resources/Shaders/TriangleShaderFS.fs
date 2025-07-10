#version 300 es
precision mediump float;

in mediump vec3 v_color;
in mediump vec2 v_uv;  // UV coordinates từ vertex shader

uniform sampler2D u_texture;  // Texture uniform

layout(location = 0) out mediump vec4 o_color;

void main()
{
	vec4 texColor = texture(u_texture, v_uv);  // Sample texture
	
	// Option 1: Pure texture (realistic) 
	o_color = texColor;
	
	// Option 2: Mix texture + vertex color (psychedelic rainbow effect)
	//vec3 finalColor = v_color * texColor.rgb;
	//o_color = vec4(finalColor, 1.0);
	
	// Option 3: Pure vertex colors (no texture)
	// o_color = vec4(v_color, 1.0);
	
	// Option 4: Debug UV coordinates (visualize UV as colors)
	//o_color = vec4(v_uv.x, v_uv.y, 0.0, 1.0);
}
