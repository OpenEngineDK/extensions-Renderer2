varying vec2 screenUV;

attribute vec2 vertex;
void main(void)
{
    screenUV = vertex * 0.5 + 0.5;

    gl_Position.xy = vertex;
    gl_Position.z = 0.0;
    gl_Position.w = 1.0;
}
