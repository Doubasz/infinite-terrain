#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vNormal;
out vec3 vViewSpacePos;
out vec3 vViewSpaceNormal;
out vec3 vPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // World space position
    vec4 worldPos = model * vec4(aPos, 1.0);
    
    // View space position (for distance calculations)
    vViewSpacePos = vec3(view * worldPos);
    
    vPosition = worldPos.xyz;

    // Transform normal to view space
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vViewSpaceNormal = normalize(normalMatrix * aNormal);
    
    // Keep original normal for reference
    vNormal = aNormal;
    
    gl_Position = projection * view * worldPos;
}