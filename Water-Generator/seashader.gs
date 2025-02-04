#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 normal;
    vec3 displacedPos;
    vec3 originalPos;
} gs_in[];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool u_showNormals; // Toggle for normal visualization mode

out vec3 FragNormal;
out vec3 FragPos;

void main()
{
    for (int i = 0; i < 3; i++) 
    {
        // Transform normal correctly to world space
        vec3 normal = normalize(mat3(transpose(inverse(model))) * gs_in[i].normal);

        // Pass per-vertex normals and positions to the fragment shader
        FragNormal = normal;
        FragPos = gs_in[i].displacedPos;

        // Emit each vertex with its own normal
        gl_Position = projection * view * model * vec4(gs_in[i].displacedPos, 1.0);
        EmitVertex();
    }

    EndPrimitive(); // Ensure each triangle gets its correct normal per vertex
}
