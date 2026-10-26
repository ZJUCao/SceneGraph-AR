#version 330 core
// Blinn-Phong lighting with one directional light, in eye coordinates.

in vec3 fPosition;
in vec3 fNormal;

uniform vec3 materialColor;
uniform vec3 lightDirection; // unit vector toward the light

out vec4 fColor;

const float AMBIENT = 0.35;
const float SPECULAR = 0.25;
const float SHININESS = 32.0;

void main()
{
    vec3 n = normalize(fNormal);
    vec3 v = normalize(-fPosition);
    // light both sides of a surface, so open meshes do not turn black from behind
    if (dot(n, v) < 0.0)
    {
        n = -n;
    }

    float diffuse = max(dot(n, lightDirection), 0.0);
    float specular = 0.0;
    if (diffuse > 0.0)
    {
        vec3 h = normalize(lightDirection + v);
        specular = pow(max(dot(n, h), 0.0), SHININESS);
    }

    vec3 color = materialColor * (AMBIENT + (1.0 - AMBIENT) * diffuse) + SPECULAR * specular;
    fColor = vec4(min(color, vec3(1.0)), 1.0);
}
