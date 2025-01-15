
struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
};

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

varying vec2 fragTexCoord;
varying vec3 fragNormal;

uniform UniformBufferObject ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragNormal = inNormal;
}