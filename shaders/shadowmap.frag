
uniform sampler2D color0;
uniform sampler2DShadow depth;

uniform sampler2DShadow shadow;
uniform mat4 lightMat;

uniform mat4 viewProjectionInverse;


varying vec2 screenUV;

float lookup(sampler2DShadow ShadowMap, vec4 ShadowCoord, vec2 v, float ShadowAmount) {
    float d = shadow2DProj(ShadowMap,ShadowCoord + vec4(v,0,0)).r;
    //    vec4 pos 
    return d < 1.0 ?  ShadowAmount : 1.0;
}


vec3 WorldPosFromDepth(in sampler2DShadow depth, in vec2 screenUV, 
                       in mat4 viewProjectionInverse){

    // Get the depth buffer value at this pixel.  
    float zOverW = shadow2D(depth, vec3(screenUV, 0.0)).x;
    // screenPos is the viewport position at this pixel in the range -1 to 1.  
    vec4 screenPos = vec4(screenUV.x * 2.0 - 1.0, 
                          screenUV.y * 2.0 - 1.0,  
                          zOverW * 2.0 - 1.0, 1.0);

    // Transform by the view-projection inverse.  
    vec4 currentPos = viewProjectionInverse * screenPos;

    // World space.
    return currentPos.xyz / currentPos.w;
}


void main(void) {

    vec3 worldPos = WorldPosFromDepth(depth, screenUV, viewProjectionInverse);
    
    vec4 coord = lightMat * vec4(worldPos,1.0);

    float sd=.05;

    float amount = 0.5;

    float d2 = shadow2D(depth, vec3(screenUV,0.0)).x;

    float l = lookup(shadow, coord, vec2(0.0, 0.0), amount);
    l += lookup(shadow, coord, vec2(sd, sd), amount);
    l += lookup(shadow, coord, vec2(sd, -sd), amount);
    l += lookup(shadow, coord, vec2(-sd, sd), amount);
    l += lookup(shadow, coord, vec2(-sd, -sd), amount);

    l += lookup(shadow, coord, vec2(0.0, sd), amount);
    l += lookup(shadow, coord, vec2(0.0, -sd), amount);
    l += lookup(shadow, coord, vec2(sd, 0.0), amount);
    l += lookup(shadow, coord, vec2(-sd, 0.0), amount);


    l /= 9.0;

    gl_FragColor = texture2D(color0, screenUV);
    
    gl_FragColor.rgb *= l;

    //gl_FragColor = vec4(l);
    //gl_FragColor = vec4(d2);
    //gl_FragColor = coord;
    //gl_FragColor.rgb = vec3(shadow2D(shadow, vec3(screenUV, 0.0)).x);

    //gl_FragColor.rg = screenUV;

    gl_FragDepth = shadow2D(depth, vec3(screenUV,0.0)).x;
}
