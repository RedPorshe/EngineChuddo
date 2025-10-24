#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    // ���������� ����� - ��������� �������� ������
    if (gl_VertexIndex == 0) {
        // ������� ������ ������ ������ ��� ��������
        fragColor = vec3(
            ubo.view[0][0] * 0.1 + 0.5,  // ���� view ������� ���������, ����� 0.5
            ubo.proj[0][0] * 0.1 + 0.5,  // �������� ��������
            ubo.model[0][0] * 0.1 + 0.5  // �������� ������
        );
    } else {
        fragColor = inColor;
    }
    
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    
    // �������������: ����������� ������� �������� �������
    // gl_Position = vec4(inPosition, 1.0); // ������� ������������� ��� �����
}