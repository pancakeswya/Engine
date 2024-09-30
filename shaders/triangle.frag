#version 450

layout(binding = 1) uniform sampler2D texSampler[MAX_TEXTURES];

layout(push_constant) uniform constants {
    uint imgIdx;
} consts;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler[consts.imgIdx], fragTexCoord);
}