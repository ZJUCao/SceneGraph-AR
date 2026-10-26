#version 330 core
// Draws a video frame or an overlay; overlays hold premultiplied alpha.

in vec2 fTexCoord;

uniform sampler2D image;

out vec4 fColor;

void main()
{
    fColor = texture(image, fTexCoord);
}
