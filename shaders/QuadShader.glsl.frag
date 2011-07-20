varying vec2 tc;
uniform vec4 color;
uniform sampler2D texIn;

void main (void) {
    gl_FragColor = texture2D(texIn, tc) * color; 
}

