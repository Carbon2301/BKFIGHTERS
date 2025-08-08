#version 300 es
precision mediump float;

in mediump vec2 v_uv;

uniform sampler2D u_texture;
uniform float u_time;
uniform vec2 u_glintDir;
uniform float u_glintWidth;
uniform float u_glintSpeed;
uniform float u_glintIntensity;

layout(location = 0) out mediump vec4 o_color;

void main() {
    vec4 base = texture(u_texture, v_uv);

    vec2 dir = normalize(u_glintDir);
    float s = dot(v_uv, dir) + u_glintSpeed * u_time;
    s = fract(s);

    float d = abs(s - 0.5);
    float band = 1.0 - smoothstep(0.0, u_glintWidth, d);

    float glint = band * u_glintIntensity * base.a;
    vec3 color = base.rgb + vec3(glint);
    o_color = vec4(clamp(color, 0.0, 1.0), base.a);
}

