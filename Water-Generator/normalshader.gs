#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

in vec3 FragPos[]; // Input from vertex shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float u_time;
uniform float sea_frequency;
uniform float sea_amplitude;

float computeWave(vec3 pos, float time) {
    return sea_amplitude * sin(pos.z * sea_frequency + time);
}

// Compute numerical normal
vec3 computeNormal(vec3 pos, float time) {
    float delta = 0.1; // Small offset for numerical derivative
    vec3 dx = vec3(delta, computeWave(pos + vec3(delta, 0.0, 0.0), time) - computeWave(pos, time), 0.0);
    vec3 dz = vec3(0.0, computeWave(pos + vec3(0.0, 0.0, delta), time) - computeWave(pos, time), delta);
    return normalize(cross(dx, dz));
}

void main()
{
    float time = u_time;

    // Compute displacement and normal
    vec3 displacedPos = FragPos[0] + vec3(0.0, computeWave(FragPos[0], time), 0.0);
    vec3 normal = computeNormal(FragPos[0], time);

    // Original vertex (start of normal line)
    gl_Position = projection * view * vec4(displacedPos, 1.0);
    EmitVertex();

    // Normal endpoint (offset along normal)
    gl_Position = projection * view * vec4(displacedPos + normal * 0.2, 1.0);
    EmitVertex();
    
    EndPrimitive();
}
