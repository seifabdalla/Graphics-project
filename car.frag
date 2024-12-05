#version 330 core

in vec3 FragPos;     // Fragment position
in vec3 Normal;      // Normal at fragment
in vec2 TexCoord;    // Texture coordinates

out vec4 FragColor;  // Final color

uniform sampler2D textureSampler; // Car texture
uniform vec3 emissiveColor;      // Emissive color to simulate glow
uniform bool isGlowing;          // Flag to check if the car is glowing

void main()
{
    // Sample the texture color
    vec4 texColor = texture(textureSampler, TexCoord);

    // If the car is glowing, apply the emissive color
    if (isGlowing) {
        FragColor = vec4(texColor.rgb + emissiveColor, texColor.a);  // Add glow effect
    } else {
        FragColor = texColor;  // Otherwise, render the car normally
    }
}
