#version 330 core

out vec4 FragColor;

in vec3 rayDir;

uniform vec3 sunDir;
uniform vec3 sunColor;

vec3 getSkyColor(vec3 dir)
{
    float t = max(dir.y * 0.5 + 0.5, 0.0);

    vec3 zenithDay = vec3(0.10, 0.25, 0.55);
    vec3 horizonDay = vec3(0.55, 0.70, 0.95);

    vec3 zenithDawn = vec3(0.02, 0.05, 0.10);
    vec3 horizonDawn = vec3(0.9, 0.45, 0.15);

    float sunElev = clamp(sunDir.y * 0.7 + 0.3, 0.0, 1.0);

    vec3 zenith = mix(zenithDawn, zenithDay, sunElev);
    vec3 horizon = mix(horizonDawn, horizonDay, sunElev);

    // blend horizon ↔ zenith
    vec3 sky = mix(horizon, zenith, t);

    // sun glow
    float sunAmount = pow(max(dot(dir, sunDir), 0.0), 400.0);
    sky += sunColor * sunAmount * 8.0;

    return sky;
}

void main() {
    FragColor = vec4(getSkyColor(normalize(rayDir)), 1.0);
}
