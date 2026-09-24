#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 1) uniform sampler2D renderTexture;
layout(set = 0, binding = 2) uniform sampler2D specularTexture;

layout(push_constant) uniform RenderTextureMode
{
    int sampleTexture;
} mode;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 albedo = texture(renderTexture, fragUV);
    vec4 specular = texture(specularTexture, fragUV);
    outColor = mode.sampleTexture != 0
        ? vec4(albedo.rgb * (0.8 + 0.2 * specular.rgb), albedo.a)
        : vec4(fragUV, 0.5, 1.0);
}