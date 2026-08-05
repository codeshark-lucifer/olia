#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;

layout(set = 0, binding = 0) uniform CameraUbo {
    mat4 view;
    mat4 proj;
} camera;

struct ObjectData {
    mat4 world_matrix;
    vec4 color_override;
};

layout(set = 0, binding = 1) readonly buffer ObjectDataSsbo {
    ObjectData objects[];
} objectBuffer;

layout(push_constant) uniform PushConstants {
    mat4 model_matrix;
    vec4 tint_color;
} push;

void main() {
    mat4 model = objectBuffer.objects[gl_InstanceIndex].world_matrix * push.model_matrix;
    gl_Position = camera.proj * camera.view * model * vec4(inPosition, 1.0);
    fragNormal = inNormal;
    fragUV = inUV;
}
