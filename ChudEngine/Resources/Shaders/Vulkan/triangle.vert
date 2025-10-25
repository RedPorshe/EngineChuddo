#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

// PUSH CONSTANTS for matrices
layout(push_constant) uniform MatrixPushConstants {
    mat4 modelMatrix;
    mat4 viewProjectionMatrix;
} matrices;

void main() {
    // Apply transformations
    mat4 mvp = matrices.viewProjectionMatrix * matrices.modelMatrix;
    gl_Position = mvp * vec4(inPosition, 1.0);
    
    fragColor = inColor;
}