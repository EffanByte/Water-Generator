#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float u_time;
uniform float sea_frequency;
uniform float sea_amplitude;

out vec3 FragPos;

float computeWave(vec3 pos, float time) {
    float wave1 = sea_amplitude * sin(pos.z * (sea_frequency + 0.1f) + pos.x * 0.3f + time);
    float wave2 = (sea_amplitude * 0.8f) * sin(pos.x * (sea_frequency + 0.15f) + time * 0.8f);
    float wave3 = (sea_amplitude * 0.9f) * sin(pos.z * (sea_frequency + 0.2f) + time * 1.5f);
    float wave4 = (sea_amplitude * 0.6f) * sin(pos.x * (sea_frequency + 0.05f) + time * 2.0f);
    return wave1 + wave2 + wave3 + wave4;
}

void main()
{
    float time = u_time;
    vec3 displacedPos = aPos;
    displacedPos.y += computeWave(aPos, time);

    FragPos = vec3(model * vec4(displacedPos, 1.0)); // Pass world position to fragment shader
    gl_Position = projection * view * model * vec4(displacedPos, 1.0);
}
