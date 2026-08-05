#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 model_matrix;
    vec4 tint_color;
} push;

void main() {
    outColor = push.tint_color;
}
