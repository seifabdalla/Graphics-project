#version 330 core

layout(location = 0) in vec3 aPos;   // Vertex position
layout(location = 1) in vec3 aNormal; // Normal
layout(location = 2) in vec2 aTexCoord; // Texture coordinates

out vec3 FragPos;   // Fragment position
out vec3 Normal;    // Normal at fragment
out vec2 TexCoord;  // Texture coordinates

uniform mat4 model;  // Model matrix
uniform mat4 view;   // View matrix
uniform mat4 projection; // Projection matrix

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));  // Transform position
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normal
    TexCoord = aTexCoord;  // Pass texture coordinates

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
