#version 330 core
in vec3 vFragPos; // World-space position
in vec3 FragNormal;
out vec4 FragColor;

// Light properties
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;

// Camera position
uniform vec3 viewPos;

// Material properties
uniform vec3 objectColor;
uniform float ambientStrength;  // Soft background light
uniform float diffuseStrength;  // Light reflection intensity
uniform float specularStrength; // Shiny reflections
uniform float shininess;        // Sharpness of specular highlight

void main()
{
    vec3 lightDir = normalize(lightPos - vFragPos);
    vec3 normal = normalize(FragNormal);
    vec3 viewDir = normalize(viewPos - vFragPos);

    // **1. Ambient Lighting**
    vec3 ambient = ambientStrength * lightColor;

    // **2. Diffuse Lighting (Lambertian Reflection)**
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * lightColor;

    // **3. Specular Lighting (Blinn-Phong Reflection)**
    vec3 halfwayDir = normalize(lightDir + viewDir); // Use halfway vector
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Combine lighting components
    vec3 result = (ambient + diffuse + specular) * objectColor * lightIntensity;

    // Output final color
    FragColor = vec4(result, 1.0);
}
