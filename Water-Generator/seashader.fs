#version 330 core
in vec3 FragPos;

out vec4 FragOutput;
in vec3 LightDir;
in vec3 Normals;
in vec3 FragColor;
void main()
{   
    FragOutput = vec4(0, 0.45, 0.65, 1.0);
}