#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float u_time;
uniform float sea_frequency;
uniform float sea_amplitude;
uniform float wave_speed;

out VS_OUT {
    vec3 normal;       // World-space normal
    vec3 displacedPos; // World-space displaced position
    vec3 originalPos;  // World-space original position
} vs_out;

void main()
{  
    // Compute multiple wave displacements
    float wave1 = sea_amplitude * sin(aPos.z * (sea_frequency + 0.1f) + aPos.x * 0.3f + u_time * wave_speed);
    float wave2 = (sea_amplitude * 0.8f) * sin(aPos.x * (sea_frequency + 0.15f) + u_time * wave_speed);
    float wave3 = (sea_amplitude * 0.9f) * sin(aPos.z * (sea_frequency + 0.2f) + u_time * wave_speed);
    float wave4 = (sea_amplitude * 0.6f) * sin(aPos.x * (sea_frequency + 0.05f) + u_time * wave_speed);
    
    // **New Additional Waves**
    float wave5 = (sea_amplitude * 0.7f) * sin((aPos.x + aPos.z) * (sea_frequency + 0.08f) + u_time * wave_speed * 1.2);
    float wave6 = (sea_amplitude * 0.5f) * sin((aPos.x - aPos.z) * (sea_frequency + 0.12f) + u_time * wave_speed * 0.8);
    float wave7 = (sea_amplitude * 0.4f) * sin((aPos.x * 0.5f + aPos.z * 0.5f) * (sea_frequency + 0.18f) + u_time * wave_speed * 1.5);
    
    float wave = wave1 + wave2 + wave3 + wave4 + wave5 + wave6 + wave7;

    // Approximate numerical derivative for normal calculation
    float epsilon = 0.1;
    float heightL = sea_amplitude * sin((aPos.z) * sea_frequency + (aPos.x - epsilon) * 0.3f + u_time * wave_speed);
    float heightR = sea_amplitude * sin((aPos.z) * sea_frequency + (aPos.x + epsilon) * 0.3f + u_time * wave_speed);
    float heightD = sea_amplitude * sin((aPos.z - epsilon) * sea_frequency + (aPos.x) * 0.3f + u_time * wave_speed);
    float heightU = sea_amplitude * sin((aPos.z + epsilon) * sea_frequency + (aPos.x) * 0.3f + u_time * wave_speed);
    
    vec3 tangentX = normalize(vec3(2.0 * epsilon, heightR - heightL, 0.0));
    vec3 tangentZ = normalize(vec3(0.0, heightU - heightD, 2.0 * epsilon));
    
    vec3 normal = normalize(cross(tangentX, tangentZ));

    // Compute world-space positions
    vec3 displacedPosition = vec3(aPos.x, aPos.y + wave, aPos.z);
    vec3 worldDisplacedPos = vec3(model * vec4(displacedPosition, 1.0));
    vec3 worldOriginalPos = vec3(model * vec4(aPos, 1.0));

    // Pass data to the geometry shader
    vs_out.normal = mat3(transpose(inverse(model))) * normal; // Convert normal to world space
    vs_out.displacedPos = worldDisplacedPos;
    vs_out.originalPos = worldOriginalPos;

    // Compute final position in clip space
    gl_Position = projection * view * vec4(worldDisplacedPos, 1.0);
}
