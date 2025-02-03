    #version 330 core
    layout(triangles) in;
    layout(line_strip, max_vertices = 6) out;

    in VS_OUT {
        vec3 normal;
        vec3 displacedPos;
        vec3 originalPos;
    } gs_in[];

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    out vec3 FragNormal;
    out vec3 FragPos;

    void main()
    {
        for (int i = 0; i < 3; i++) 
        {
            // Emit original vertex
            gl_Position = projection * view * model * vec4(gs_in[i].displacedPos, 1.0);
            EmitVertex();

            // Emit vertex displaced along normal (visualizing normal direction)
            gl_Position = projection * view * model * vec4(gs_in[i].displacedPos + gs_in[i].normal * 0.2, 1.0);
            EmitVertex();

            EndPrimitive();
        }
    }
