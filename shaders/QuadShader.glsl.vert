varying vec2 tc;
uniform vec2 dims;
attribute vec2 vertex, tcIn;

void main()
{
    tc = tcIn;
    gl_Position.xy = (vertex / dims) * 2.0 - 1.0;
}
