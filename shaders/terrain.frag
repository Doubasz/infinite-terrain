#version 330 core

in vec3 vFragPos;
out vec4 FragColor;

void main()
{
    float h = vFragPos.y;

    // Customize height range
    float minH = 0.0;
    float maxH = 120.0;
    float t = clamp((h - minH) / (maxH - minH), 0.0, 1.0);

    // Four height colors
    vec3 c1 = vec3(0.25, 0.15, 0.05);  // dark brown (very low)
    vec3 c2 = vec3(0.10, 0.70, 0.10);  // green (mid)
    vec3 c3 = vec3(0.50, 0.50, 0.50);  // gray rock
    vec3 c4 = vec3(1.00, 1.00, 1.00);  // snow (top)

    vec3 color;

    // Blend between four layers
    if (t < 0.33) {
        float k = smoothstep(0.0, 0.33, t);
        color = mix(c1, c2, k);
    }
    else if (t < 0.66) {
        float k = smoothstep(0.33, 0.66, t);
        color = mix(c2, c3, k);
    }
    else {
        float k = smoothstep(0.66, 1.0, t);
        color = mix(c3, c4, k);
    }

    FragColor = vec4(color, 1.0);
}
