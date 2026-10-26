#version 330 core
// Scene graph meshes: transform to eye coordinates for lighting, then project.

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vNormal;

uniform mat4 projection;
uniform mat4 modelview;
uniform mat3 normalmatrix; // inverse transpose of the upper 3x3 of modelview

out vec3 fPosition;
out vec3 fNormal;

void main()
{
    vec4 eyePosition = modelview * vPosition;
    fPosition = eyePosition.xyz;
    fNormal = normalmatrix * vNormal.xyz;
    gl_Position = projection * eyePosition;
}
