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

    float vertexTime = u_time;

// Compute intermediate arguments for the wave functions
float A1 = aPos.z * (sea_frequency + 0.1f) + aPos.x * 0.3f + vertexTime;
float A2 = aPos.x * (sea_frequency + 0.15f) + vertexTime * wave_speed;
float A3 = aPos.z * (sea_frequency + 0.2f) + vertexTime * wave_speed;
float A4 = aPos.x * (sea_frequency + 0.05f) + vertexTime * wave_speed;
float A5 = (aPos.x + aPos.z) * (sea_frequency + 0.08f) + vertexTime * wave_speed * 1.2;
float A6 = (aPos.x - aPos.z) * (sea_frequency + 0.12f) + vertexTime * wave_speed * 0.8;
float A7 = (aPos.x * 0.5f + aPos.z * 0.5f) * (sea_frequency + 0.18f) + vertexTime * wave_speed * 1.5;

// Compute wave displacements using the exponent of sine
float wave1 = sea_amplitude * exp(sin(A1));
float wave2 = (sea_amplitude * 0.8f) * exp(sin(A2));
float wave3 = (sea_amplitude * 0.9f) * exp(sin(A3));
float wave4 = (sea_amplitude * 0.6f) * exp(sin(A4));
float wave5 = (sea_amplitude * 0.7f) * exp(sin(A5));
float wave6 = (sea_amplitude * 0.5f) * exp(sin(A6));
float wave7 = (sea_amplitude * 0.4f) * exp(sin(A7));

float wave = wave1 + wave2 + wave3 + wave4 + wave5 + wave6 + wave7;

// Compute partial derivative with respect to x (dHx)
// For each wave, d/dx[exp(sin(A))] = exp(sin(A)) * cos(A) * dA/dx
float dHx = 
      sea_amplitude * exp(sin(A1)) * cos(A1) * 0.3
    + (sea_amplitude * 0.8f) * exp(sin(A2)) * cos(A2) * (sea_frequency + 0.15f)
    // wave3 has no dependence on x (A3 depends only on aPos.z), so no term for wave3
    + (sea_amplitude * 0.6f) * exp(sin(A4)) * cos(A4) * (sea_frequency + 0.05f)
    + (sea_amplitude * 0.7f) * exp(sin(A5)) * cos(A5) * (sea_frequency + 0.08f)
    + (sea_amplitude * 0.5f) * exp(sin(A6)) * cos(A6) * (sea_frequency + 0.12f)
    + (sea_amplitude * 0.4f) * exp(sin(A7)) * cos(A7) * (0.5f * (sea_frequency + 0.18f));

// Compute partial derivative with respect to z (dHz)
float dHz = 
      sea_amplitude * exp(sin(A1)) * cos(A1) * (sea_frequency + 0.1f)
    // wave2 has no dependence on z (A2 depends only on aPos.x), so no term for wave2
    + (sea_amplitude * 0.9f) * exp(sin(A3)) * cos(A3) * (sea_frequency + 0.2f)
    // wave4 has no dependence on z (A4 depends only on aPos.x), so no term for wave4
    + (sea_amplitude * 0.7f) * exp(sin(A5)) * cos(A5) * (sea_frequency + 0.08f)
    + (sea_amplitude * 0.5f) * exp(sin(A6)) * cos(A6) * (-(sea_frequency + 0.12f))
    + (sea_amplitude * 0.4f) * exp(sin(A7)) * cos(A7) * (0.5f * (sea_frequency + 0.18f));

    // Compute normal using the gradient
    vec3 updatedNormal = normalize(vec3(-dHx, 1.0, -dHz));


    // Compute world-space positions
    vec3 displacedPosition = vec3(aPos.x, aPos.y + wave, aPos.z);
    vec3 worldDisplacedPos = vec3(model * vec4(displacedPosition, 1.0));
    vec3 worldOriginalPos = vec3(model * vec4(aPos, 1.0));

    FragNormal = normalize(normalMatrix * updatedNormal);
    vFragPos = vec3(model * vec4(displacedPosition, 1.0));
    // Compute final position in clip space
    gl_Position = projection * view * vec4(worldDisplacedPos, 1.0);
}
