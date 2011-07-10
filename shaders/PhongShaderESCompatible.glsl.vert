struct LightSource {
    vec4 position;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

uniform Material frontMaterial;
uniform LightSource lightSource[NUM_LIGHTS];

varying vec3 norm, eyeVec;
varying vec3 lightDir[NUM_LIGHTS];
varying float dist[NUM_LIGHTS];

// standard vertex attributes
attribute vec3 vertex;
attribute vec3 normal;
//attribute vec3 color;
attribute vec3 tangent, bitangent;

attribute vec2 texCoord0;
attribute vec2 texCoord1;
attribute vec2 texCoord2;
attribute vec2 texCoord3;
attribute vec2 texCoord4;
attribute vec2 texCoord5;
varying vec2 texCoord[6];

uniform mat4 modelViewMatrix, modelViewProjectionMatrix;
uniform mat3 normalMatrix;

//#undef BUMP_MAP

void main()
{
    vec3 vert = (modelViewMatrix * vec4(vertex,1.0)).xyz;
    vec3 n = normalize(normalMatrix * normal);
    eyeVec = normalize(-vert);
#ifdef BUMP_MAP
    vec3 t = normalize(normalMatrix * tangent);
    //vec3 b = cross(n,t);
    vec3 b = normalize(normalMatrix * bitangent);

    vec3 ev = eyeVec;
	//transform eye vector into tangent space
    eyeVec.x = dot(ev,t);
 	eyeVec.y = dot(ev,b);
    eyeVec.z = dot(ev,n);
#endif

    for (int i = 0; i < NUM_LIGHTS; ++i) {

        if (lightSource[i].position.w == 0.0) {// if directional light
            lightDir[i] = 
                lightSource[i].position.xyz; 
        }
        else { // else assume positional light
            vec3 lv = lightSource[i].position.xyz - vert;
            dist[i] = length(lv);
            lightDir[i] =
                normalize(lv);
        }
#ifdef BUMP_MAP
        // transform lightdir into tangent space
        vec3 ld = lightDir[i];
        ld.x = dot(lightDir[i],t);
        ld.y = dot(lightDir[i],b);
        ld.z = dot(lightDir[i],n);
        lightDir[i] = normalize(ld);
#endif
    }

    // interpolate texture coordinates
    texCoord[0] = texCoord0;
    texCoord[1] = texCoord1; 
    texCoord[2] = texCoord2; 
    texCoord[3] = texCoord3; 
    texCoord[4] = texCoord4; 
    texCoord[5] = texCoord5; 

    gl_Position = modelViewProjectionMatrix * vec4(vertex, 1.0);
    norm = n;
}
