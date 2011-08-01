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
uniform vec4 globalAmbient;

varying vec3 norm, eyeVec;
varying vec3 lightDir[NUM_LIGHTS];
varying float dist[NUM_LIGHTS];

#ifdef USE_TEXTURES
varying vec2 uv[NUM_TEXTURES];
#endif

uniform sampler2D ambientMap, diffuseMap, specularMap, opacityMap;
uniform sampler2D bumpMap;

//#undef BUMP_MAP
void main (void)
{
#ifdef BUMP_MAP
    vec3 n = normalize(texture2D(bumpMap, uv[BUMP_INDEX].st).xyz * 2.0 - 1.0);
#else
    vec3 n = normalize(norm);
#endif  
    
    vec4 color = 
        globalAmbient * 
#ifdef AMBIENT_MAP
        texture2D(ambientMap, uv[AMBIENT_INDEX].st);
#else
        frontMaterial.ambient;
#endif

#ifdef OPACITY_MAP  
    if (texture2D(opacityMap, uv[OPACITY_INDEX].st).a < .5) 
        discard;
#endif 

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        float att = 1.0;
        if (lightSource[i].position.w == 1.0) {// if point light
             att /=
                 lightSource[i].constantAttenuation +
                 lightSource[i].linearAttenuation * dist[i] +
                 lightSource[i].quadraticAttenuation * dist[i] * dist[i];
        }

        color +=
#ifdef AMBIENT_MAP
            texture2D(ambientMap, uv[AMBIENT_INDEX].st) *
#else
            frontMaterial.ambient *
#endif
            lightSource[i].ambient;

        vec3 l = normalize(lightDir[i]);
        vec3 e = normalize(eyeVec);

        float lambertTerm = dot(n,l);
        if (lambertTerm > 0.0)
            {
                color +=
                    att *
                    lightSource[i].diffuse *

#ifdef DIFFUSE_MAP
 //                    texture2D(diffuseMap, uv[DIFFUSE_INDEX].st) *
#else
                    frontMaterial.diffuse *
#endif
                    lambertTerm;
                
                vec3 r = reflect(-l, n);
                float specular = pow(max(dot(r, e), 0.0),
                                     frontMaterial.shininess);
                color +=
                    att *
                    lightSource[i].specular *
#ifdef SPECULAR_MAP
                    texture2D(specularMap, uv[DIFFUSE_INDEX].st) *
#else
                    frontMaterial.specular *
#endif
                    specular;
            }
    }
#ifdef DIFFUSE_MAP  
    // Weight the final color with the diffuse map.
    // This resembles the gl fixed function pipeline way.
    color *= texture2D(diffuseMap, uv[DIFFUSE_INDEX].st);
#endif 

    gl_FragColor = color;
    //gl_FragColor.rgb = gl_FragCoord.w * 200;
    //gl_FragColor.rgb = n; 
    //gl_FragColor.rgb = normalize(lightDir[0]);
}

