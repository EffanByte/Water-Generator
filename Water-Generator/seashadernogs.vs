#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float u_time;
uniform float sea_frequency;
uniform float sea_amplitude;

out vec3 FragNormal;
out vec3 vFragPos;
void main()
{ 
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    // Use gl_VertexID to compute unique time for each vertex
    float vertexTime = u_time;

    // Compute wave displacement
    float wave1 = sea_amplitude * sin(aPos.z * (sea_frequency + 0.1f) + aPos.x * 0.3f + vertexTime);
    float wave2 = (sea_amplitude * 0.8f) * sin(aPos.x * (sea_frequency + 0.15f) + vertexTime * 0.8f);
    float wave3 = (sea_amplitude * 0.9f) * sin(aPos.z * (sea_frequency + 0.2f) + vertexTime * 1.5f);
    float wave4 = (sea_amplitude * 0.6f) * sin(aPos.x * (sea_frequency + 0.05f) + vertexTime * 2.0f);
    float wave = wave1 + wave2 + wave3 + wave4;

    // Compute numerical derivative for normal approximation
    float delta = 0.1;
    float wave_dx = sea_amplitude * sin((aPos.z + delta) * (sea_frequency + 0.1f) + (aPos.x + delta) * 0.3f + vertexTime) - wave;
    float wave_dz = sea_amplitude * sin((aPos.x + delta) * (sea_frequency + 0.15f) + vertexTime * 0.8f) - wave;

    vec3 tangentX = normalize(vec3(delta, wave_dx, 0.0));
    vec3 tangentZ = normalize(vec3(0.0, wave_dz, delta));
    vec3 updatedNormal = normalize(cross(tangentX, tangentZ));

    // Compute world-space positions
    vec3 displacedPosition = vec3(aPos.x, aPos.y + wave, aPos.z);
    vec3 worldDisplacedPos = vec3(model * vec4(displacedPosition, 1.0));
    vec3 worldOriginalPos = vec3(model * vec4(aPos, 1.0));

    // Pass updated normal to the geometry shader
    FragNormal = normalize(normalMatrix * updatedNormal);
    vFragPos = vec3(model * vec4(displacedPosition, 1.0));
    // Compute final position in clip space
    gl_Position = projection * view * vec4(worldDisplacedPos, 1.0);
}
