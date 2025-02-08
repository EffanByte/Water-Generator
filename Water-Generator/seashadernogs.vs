#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float u_time;
uniform float sea_frequency;
uniform float sea_amplitude;
uniform float wave_speed;

out vec3 FragNormal;
out vec3 vFragPos;
void main()
{ 
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    // Use gl_VertexID to compute unique time for each vertex
    float vertexTime = u_time;

    // Compute wave displacement
    float wave1 = sea_amplitude * sin(aPos.z * (sea_frequency + 0.1f) + aPos.x * 0.3f + vertexTime);
    float wave2 = (sea_amplitude * 0.8f) * sin(aPos.x * (sea_frequency + 0.15f) + vertexTime * wave_speed);
    float wave3 = (sea_amplitude * 0.9f) * sin(aPos.z * (sea_frequency + 0.2f) + vertexTime * wave_speed);
    float wave4 = (sea_amplitude * 0.6f) * sin(aPos.x * (sea_frequency + 0.05f) + vertexTime * wave_speed);
        // **New Additional Waves**
    float wave5 = (sea_amplitude * 0.7f) * sin((aPos.x + aPos.z) * (sea_frequency + 0.08f) + u_time * wave_speed * 1.2);
    float wave6 = (sea_amplitude * 0.5f) * sin((aPos.x - aPos.z) * (sea_frequency + 0.12f) + u_time * wave_speed * 0.8);
    float wave7 = (sea_amplitude * 0.4f) * sin((aPos.x * 0.5f + aPos.z * 0.5f) * (sea_frequency + 0.18f) + u_time * wave_speed * 1.5);
    
    float wave = wave1 + wave2 + wave3 + wave4 + wave5 + wave6 + wave7;

    // **Compute Partial Derivatives (dH/dx and dH/dz)**
    float dHx = sea_amplitude * (0.3f * cos(aPos.z * (sea_frequency + 0.1f) + aPos.x * 0.3f + u_time)) +
                (sea_amplitude * 0.8f) * ((sea_frequency + 0.15f) * cos(aPos.x * (sea_frequency + 0.15f) + u_time * wave_speed)) +
                (sea_amplitude * 0.6f) * ((sea_frequency + 0.05f) * cos(aPos.x * (sea_frequency + 0.05f) + u_time * wave_speed)) +
                (sea_amplitude * 0.7f) * ((sea_frequency + 0.08f) * cos((aPos.x + aPos.z) * (sea_frequency + 0.08f) + u_time * wave_speed * 1.2)) +
                (sea_amplitude * 0.5f) * ((sea_frequency + 0.12f) * cos((aPos.x - aPos.z) * (sea_frequency + 0.12f) + u_time * wave_speed * 0.8)) +
                (sea_amplitude * 0.4f * 0.5f) * ((sea_frequency + 0.18f) * cos((aPos.x * 0.5f + aPos.z * 0.5f) * (sea_frequency + 0.18f) + u_time * wave_speed * 1.5));

    float dHz = sea_amplitude * ((sea_frequency + 0.1f) * cos(aPos.z * (sea_frequency + 0.1f) + aPos.x * 0.3f + u_time)) +
                (sea_amplitude * 0.9f) * ((sea_frequency + 0.2f) * cos(aPos.z * (sea_frequency + 0.2f) + u_time * wave_speed)) +
                (sea_amplitude * 0.7f) * ((sea_frequency + 0.08f) * cos((aPos.x + aPos.z) * (sea_frequency + 0.08f) + u_time * wave_speed * 1.2)) +
                (-sea_amplitude * 0.5f) * ((sea_frequency + 0.12f) * cos((aPos.x - aPos.z) * (sea_frequency + 0.12f) + u_time * wave_speed * 0.8)) +
                (sea_amplitude * 0.4f * 0.5f) * ((sea_frequency + 0.18f) * cos((aPos.x * 0.5f + aPos.z * 0.5f) * (sea_frequency + 0.18f) + u_time * wave_speed * 1.5));

    // Compute normal using the gradient
    vec3 updatedNormal = normalize(vec3(-dHx, 1.0, -dHz));

 //   vec3 updatedNormal = normalize(cross(tangentX, tangentZ));

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
