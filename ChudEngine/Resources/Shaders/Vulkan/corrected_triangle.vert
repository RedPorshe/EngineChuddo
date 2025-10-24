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
    // Отладочный вывод - проверьте значения матриц
    if (gl_VertexIndex == 0) {
        // Выводим первую строку матриц для проверки
        fragColor = vec3(
            ubo.view[0][0] * 0.1 + 0.5,  // Если view матрица идентична, будет 0.5
            ubo.proj[0][0] * 0.1 + 0.5,  // Проверка проекции
            ubo.model[0][0] * 0.1 + 0.5  // Проверка модели
        );
    } else {
        fragColor = inColor;
    }
    
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    
    // Альтернативно: используйте простую тестовую позицию
    // gl_Position = vec4(inPosition, 1.0); // Уберите трансформации для теста
}