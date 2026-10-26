#version 330 core
// A quad that fills the viewport, generated from gl_VertexID (draw 4 vertices as a triangle
// strip). Texture row 0 is the top of the image, as in OpenCV.

out vec2 fTexCoord;

void main()
{
    vec2 position = vec2((gl_VertexID & 1) == 0 ? -1.0 : 1.0,
                         (gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    fTexCoord = vec2(0.5 * position.x + 0.5, 0.5 - 0.5 * position.y);
    gl_Position = vec4(position, 0.0, 1.0);
}
