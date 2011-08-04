// OpenGL renderer context
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers2/OpenGL/GLContext.h>
#include <Resources/ITexture2D.h>
#include <Resources/ICubemap.h>

#include <Display2/ICanvas.h>
#include <Display2/Canvas2D.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using namespace Resources;
using Resources2::Uniform;
using Resources2::Shader;
using Display2::ICanvas;

using namespace std;

GLContext::GLContext()
    : init(false)
    , fboSupport(false)
    , vboSupport(false)
    , shaderSupport(false) 
{    
}

GLContext::~GLContext() {

}

void GLContext::Init() {
    if (init) return;
    // Initialize the "OpenGL Extension Wrangler" library
    GLenum err = glewInit();
    if (err!=GLEW_OK)
        logger.error << "GLEW: "
                     << (const char*)glewGetErrorString(err)
                     << logger.end;
    else {
        logger.info << "OpenGL: "
                    << (const char*)glGetString(GL_VERSION)
                    << " GLEW: "
                    << (const char*)glewGetString(GLEW_VERSION)
                    << logger.end;

		if (glewIsSupported("GL_VERSION_2_0")) {
            glslversion = GLSL_20;
			logger.info << "Using OpenGL version 2.0 with GLSL: "
                        << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)
                        << logger.end;
		}
		else if (glewIsSupported("GL_VERSION_1_4") &&
                 GLEW_ARB_vertex_shader &&
                 GLEW_ARB_fragment_shader) {
            glslversion = GLSL_14;
			logger.info << "Using OpenGL version 1.4 with shaders as extensions"
                        << logger.end;
		}
		else {
            glslversion = GLSL_NONE;
            logger.info << "GLSL not supported - shaders are disabled"
                        << logger.end;
		}
    }

    fboSupport = glewGetExtension("GL_EXT_framebuffer_object") == GL_TRUE;
    vboSupport = glewIsSupported("GL_VERSION_2_0");
    shaderSupport = glewIsSupported("GL_VERSION_2_0");
    init = true;
}

bool GLContext::FBOSupport() {
    return fboSupport;
}

bool GLContext::VBOSupport() {
    return vboSupport;
}

bool GLContext::ShaderSupport() {
    return shaderSupport;
}
    
GLint GLContext::GLInternalColorFormat(ColorFormat f){
    switch (f) {
    case ALPHA:
        return GL_ALPHA;
    case LUMINANCE: 
        return GL_LUMINANCE;
    case LUMINANCE_ALPHA: 
        return GL_LUMINANCE_ALPHA;
    case BGR:
    case RGB: 
        return GL_RGB;
    case BGRA: 
    case RGBA: 
        return GL_RGBA;
    case ALPHA_COMPRESSED: return GL_COMPRESSED_ALPHA;
    case LUMINANCE_COMPRESSED: return GL_COMPRESSED_LUMINANCE;
    case LUMINANCE32F: return GL_R32F;
    case LUMINANCE_ALPHA_COMPRESSED: return GL_COMPRESSED_LUMINANCE_ALPHA;
    case RGB_COMPRESSED: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case RGBA_COMPRESSED: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case RGB32F: return GL_RGB32F;
    case RGBA32F: return GL_RGBA32F;
    case DEPTH: return GL_DEPTH_COMPONENT;
    default: 
        // todo: should throw exception here ? ...
        logger.warning << "Unsupported color format: " << f << logger.end;
        logger.warning << "Defaulting to RGBA." << logger.end;
    }
    return GL_RGBA;
}

GLenum GLContext::GLColorFormat(ColorFormat f){
    switch (f) {
    case ALPHA:
    case ALPHA_COMPRESSED:
        return GL_ALPHA;
    case LUMINANCE: 
    case LUMINANCE_COMPRESSED: 
    case LUMINANCE32F:
        return GL_LUMINANCE;
    case LUMINANCE_ALPHA: 
    case LUMINANCE_ALPHA_COMPRESSED: 
        return GL_LUMINANCE_ALPHA;
    case RGB: 
    case RGB32F: 
    case RGB_COMPRESSED: 
        return GL_RGB;
    case RGBA: 
    case RGBA_COMPRESSED: 
    case RGBA32F: 
        return GL_RGBA;
    case BGR: 
        return GL_BGR;
    case BGRA: 
        return GL_BGRA;
    case DEPTH: 
        return GL_DEPTH_COMPONENT;
    default: 
        // todo: should throw exception here ? ...
        logger.warning << "Unsupported color format: " << f << logger.end;
        logger.warning << "Defaulting to RGBA." << logger.end;
    }
    return GL_RGBA;
}

unsigned int GLContext::GLTypeSize(Type t){
    switch(t){
    case Types::UBYTE:
        return sizeof(GLubyte);
    case Types::SBYTE:
        return sizeof(GLbyte);
    case Types::USHORT:
        return sizeof(GLushort);
    case Types::SHORT:
        return sizeof(GLshort);
    case Types::UINT:
        return sizeof(GLuint);
    case Types::INT:
        return sizeof(GLint);
    case Types::FLOAT:
        return sizeof(GLfloat);
    case Types::DOUBLE:
        return sizeof(GLdouble);
    default:
        //case Types::NOTYPE:
        throw Exception("GLTypeSize: Unknown type.");
    }
    return sizeof(GLshort);
}

GLenum GLContext::GLAccessType(BlockType b, UpdateMode u){
    if (u == STATIC){
        switch (b){
        case PIXEL_PACK:
            return GL_STATIC_COPY;
        default:
            return GL_STATIC_DRAW;
        }
    }else if (u == DYNAMIC){
        switch (b){
        case PIXEL_PACK:
            return GL_DYNAMIC_COPY;
        default:
            return GL_DYNAMIC_DRAW;
        }
    }
    return GL_STATIC_DRAW;
}

// ------- Canvas -------

GLuint GLContext::LoadCanvas(ICanvas* can) {
#if OE_SAFE
    if (can == NULL) throw Exception("Cannot load NULL canvas.");
#endif
    GLuint texid; 
    glGenTextures(1, &texid);
    CHECK_FOR_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D, texid);
    CHECK_FOR_GL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    GLint internalFormat = GLInternalColorFormat(can->GetColorFormat());
    GLenum format = GLColorFormat(can->GetColorFormat());

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                 can->GetWidth(), can->GetHeight(), 0, format, 
                 GL_UNSIGNED_BYTE, NULL);
    CHECK_FOR_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D, 0);
    return texid;
}

GLuint GLContext::LookupCanvas(ICanvas* can) {
    map<ICanvas*, GLuint>::iterator it = canvases.find(can);
    if (it != canvases.end())
        return it->second;
    GLuint id = LoadCanvas(can);
    canvases[can] = id;
    return id;
}

GLuint GLContext::LookupCanvas(Canvas2D* can) {
    GLuint id = LookupTexture(can->GetTexture().get());
    canvases[can] = id;
    return id;
}

// ------- Cubemap -------

GLuint GLContext::LoadCubemap(ICubemap* cubemap) {
#if OE_SAFE
    if (cubemap == NULL) 
        throw Exception("Cannot load NULL cubemap.");
#endif

    GLuint texid;
    glGenTextures(1, &texid);
    CHECK_FOR_GL_ERROR();

    cubemap->SetID(texid); // deprecated nasty stuff
    glBindTexture(GL_TEXTURE_CUBE_MAP, texid);
    CHECK_FOR_GL_ERROR();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_FOR_GL_ERROR();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    CHECK_FOR_GL_ERROR();

    // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    // CHECK_FOR_GL_ERROR();

    bool mipmapped = cubemap->IsMipmapped();
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, mipmapped ? GL_TRUE : GL_FALSE);
    switch(cubemap->GetFiltering()){
    case NONE:
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    default:
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    CHECK_FOR_GL_ERROR();

    // Only support for RGBA32
    for (int i = 0; i < 6; ++i){
        for (int m = 0; m < cubemap->MipmapCount(); ++m)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m,
                         GL_RGBA, cubemap->Width(m), cubemap->Height(m), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         cubemap->GetRawData((ICubemap::Face)(ICubemap::POSITIVE_X + i), m));
        CHECK_FOR_GL_ERROR();
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return texid;
}

GLuint GLContext::LookupCubemap(ICubemap* cubemap) {
    map<ICubemap*, GLuint>::iterator it = cubemaps.find(cubemap);
    if (it != cubemaps.end())
        return it->second;
    GLuint id = LoadCubemap(cubemap);
    cubemaps[cubemap] = id;
    return id;
}
 

// ------- Texture -------
void GLContext::SetupTexParameters(ITexture2D* tex){
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_FOR_GL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex->GetWrapping());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex->GetWrapping());

    if (tex->UseMipmapping()) {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->GetFiltering());
    } 
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
        if (tex->GetFiltering() == NONE)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    if (tex->GetFiltering() == NONE)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECK_FOR_GL_ERROR();
}


GLuint GLContext::LoadTexture(ITexture2D* tex) {
#if OE_SAFE
    if (tex == NULL) throw Exception("Cannot load NULL texture.");
#endif
    // signal we need the texture data if not loaded.
    bool loaded = true;
    if (tex->GetVoidDataPtr() == NULL){
        loaded = false;
        tex->Load(); //@todo: what the #@!%?
    }
    
    GLuint texid; 
    glGenTextures(1, &texid);
    CHECK_FOR_GL_ERROR();

    tex->SetID(texid); // this operation is deprecated! Get texture id by querying the GLContext.
    glBindTexture(GL_TEXTURE_2D, texid);
    CHECK_FOR_GL_ERROR();

    SetupTexParameters(tex);
    
    GLint internalFormat = GLInternalColorFormat(tex->GetColorFormat());
    GLenum colorFormat = GLColorFormat(tex->GetColorFormat());

    glTexImage2D(GL_TEXTURE_2D,
                 0, // mipmap level
                 internalFormat,
                 tex->GetWidth(),
                 tex->GetHeight(),
                 0, // border
                 colorFormat,
                 tex->GetType(),
                 tex->GetVoidDataPtr());
    CHECK_FOR_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D, 0);

    // Return the texture in the state we got it.
    if (!loaded)
        tex->Unload();

    return texid;
}

GLuint GLContext::LookupTexture(ITexture2D* tex) {
    map<ITexture2D*, GLuint>::iterator it = textures.find(tex);
    if (it != textures.end()) {
        return it->second;
    }
    GLuint id = LoadTexture(tex);
    tex->ChangedEvent().Attach(*this);

    textures[tex] = id;
    return id;
}

// ------- VBO -------
GLuint GLContext::LoadVBO(IDataBlock* db) {
#if OE_SAFE
    if (!vboSupport) throw Exception("VBOs not supported.");
    if (db == NULL) throw Exception("Cannot bind NULL data block.");
    if (db->GetVoidDataPtr() == NULL) throw Exception("Cannot bind data block with no data.");
#endif
    GLuint id;
    glGenBuffers(1, &id);
    CHECK_FOR_GL_ERROR();
    
    db->SetID(id); // this operation is deprecated! Get vbo id by querying the GLContext.
    glBindBuffer(db->GetBlockType(), id);
    CHECK_FOR_GL_ERROR();
    
    unsigned int size = GLTypeSize(db->GetType()) * db->GetSize() * db->GetDimension();
    GLenum access = GLAccessType(db->GetBlockType(), db->GetUpdateMode());
        
    glBufferData(db->GetBlockType(), 
                 size,
                 db->GetVoidDataPtr(), access); 
    glBindBuffer(db->GetBlockType(), 0);
   
    if (db->GetUnloadPolicy() == UNLOAD_AUTOMATIC)
        db->Unload();
    return id;
}

GLuint GLContext::LookupVBO(IDataBlock* db) {
    map<IDataBlock*, GLuint>::iterator it = vbos.find(db);
    if (it != vbos.end())
        return (*it).second;
    GLuint id = LoadVBO(db);
    db->ChangedEvent().Attach(*this);
    vbos[db] = id;
    return id;
}


// ------- Shader -------
void PrintProgramInfoLog(GLuint program) {
    GLint infologLength = 0, charsWritten = 0;
    GLchar* infoLog;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
        infoLog = (GLchar *)malloc(infologLength);
        if (infoLog==NULL) {
            logger.error << "Could not allocate InfoLog buffer" << logger.end;
            return;
        }
        glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
        logger.info << "Program InfoLog:\n \"" << infoLog << "\"" << logger.end;
        free(infoLog);
    }
}

GLuint GLContext::LoadShader(Shader* shad) {
#if OE_SAFE
    if (!shaderSupport) throw Exception("Shaders not supported.");
    if (shad == NULL) throw Exception("Cannot load NULL shader.");
#endif

    GLuint shaderId = glCreateProgram();
    GLuint vertexId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
    
#if OE_SAFE
    if (shaderId == 0)
        throw Exception("Failed to create shader program.");
    if (vertexId == 0)
        throw Exception("Failed to create vertex shader.");
    if (fragmentId == 0)
        throw Exception("Failed to create fragment shader.");
#endif
    glAttachShader(shaderId, vertexId);
    glAttachShader(shaderId, fragmentId);
    CHECK_FOR_GL_ERROR();

#if OE_IOS
    string iosHeader = "precision mediump float;\n";
#endif

    // compile vertex shader
    const GLchar* shaderBits[1];
    string vertexShader = shad->GetVertexShader();
#if OE_IOS
    vertexShader = iosHeader + vertexShader;
#endif
    shaderBits[0] = vertexShader.c_str();
    glShaderSource(vertexId, 1, shaderBits, NULL);
    glCompileShader(vertexId);

#if OE_SAFE
    GLint  compiled;
    glGetShaderiv(vertexId, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLsizei bufsize;
        const int maxBufSize = 1024;
        char buffer[maxBufSize];
        glGetShaderInfoLog(vertexId, maxBufSize, &bufsize, buffer);
        logger.error << "compile errors:\n" << buffer << logger.end;
        logger.error << "in shader:\n" << vertexShader << logger.end;
        throw Exception("Failed to compile vertex shader.");
    }
#endif

    // compile fragment shader
    string fragmentShader = shad->GetFragmentShader();;
#if OE_IOS
    fragmentShader = iosHeader + fragmentShader;
#endif
    shaderBits[0] = fragmentShader.c_str();
    glShaderSource(fragmentId, 1, shaderBits, NULL);
    glCompileShader(fragmentId);
    glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &compiled);
#if OE_SAFE
    glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLsizei bufsize;
        const int maxBufSize = 1024;
        char buffer[maxBufSize];
        glGetShaderInfoLog(fragmentId, maxBufSize, &bufsize, buffer);
        logger.error << "compile errors: " << buffer << logger.end;
        throw Exception("Failed to compile fragment shader.");
    }
#endif

    // Link the program object and print out the info log
    glLinkProgram(shaderId);
#if OE_SAFE
    GLint linked;
    glGetProgramiv(shaderId, GL_LINK_STATUS, &linked);
    if(linked == GL_FALSE) {
        PrintProgramInfoLog(shaderId);
        throw Exception("Failed to link shader program");
    }
#endif
    CHECK_FOR_GL_ERROR();
    return shaderId;
}

GLContext::GLShader GLContext::ResolveLocations(GLuint id, Shader* shad) {
    GLContext::GLShader glshader;
    glshader.id = id;
    Shader::UniformIterator uni_it = shad->UniformsBegin();
    for (; uni_it != shad->UniformsEnd(); ++uni_it) {
        GLint loc = glGetUniformLocation(id, uni_it->first.c_str());
        if (loc == -1) continue;
        glshader.uniforms[&uni_it->second] = loc;
    }

    Shader::AttributeIterator attr_it = shad->AttributesBegin();
    for (; attr_it != shad->AttributesEnd(); ++attr_it) {
        GLint loc = glGetAttribLocation(id, attr_it->first.c_str());
        if (loc == -1) continue;
        glshader.attributes[attr_it->second] = loc;        
    }

    Shader::Texture2DIterator tex_it = shad->Textures2DBegin();
    for (; tex_it != shad->Textures2DEnd(); ++tex_it) {
        GLint loc = glGetUniformLocation(id, tex_it->first.c_str());
        if (loc == -1) continue;
        glshader.textures[tex_it->second] = loc;        
    }

    Shader::CubemapIterator cube_it = shad->CubemapsBegin();
    for (; cube_it != shad->CubemapsEnd(); ++cube_it) {
        GLint loc = glGetUniformLocation(id, cube_it->first.c_str());
        if (loc == -1) continue;
        glshader.cubemaps[cube_it->second] = loc;        
    }

    return glshader;
}

GLContext::GLShader GLContext::LookupShader(Shader* shad) {
    map<Shader*, GLShader>::iterator it = shaders.find(shad);
    if (it != shaders.end())
        return (*it).second;
    GLuint id = LoadShader(shad);
    GLContext::GLShader glshader = ResolveLocations(id, shad);
    shaders[shad] = glshader;
    shad->ChangedEvent().Attach(*this);
    return glshader;
}

void GLContext::ReleaseTextures() {
    map<ITexture2D*, GLuint>::iterator it = textures.begin();
     for (; it != textures.end(); ++it) {
         glDeleteTextures(1, &it->second);
         it->first->ChangedEvent().Detach(*this);
     }

     map<ICanvas*, GLuint>::iterator it2 = canvases.begin();
     for (; it2 != canvases.end(); ++it2) {
         glDeleteTextures(1, &it2->second);
     }

     map<ICubemap*, GLuint>::iterator it3 = cubemaps.begin();
     for (; it3 != cubemaps.end(); ++it3) {
         glDeleteTextures(1, &it3->second);
     }

     textures.clear();
     canvases.clear();
     cubemaps.clear();
}

void GLContext::ReleaseVBOs() {
    map<IDataBlock*, GLuint>::iterator it = vbos.begin();
    for (; it != vbos.end(); ++it) {
        glDeleteBuffers(1, &it->second);
        it->first->ChangedEvent().Detach(*this);
    }
    vbos.clear();
}

void GLContext::ReleaseShaders() {
    map<Shader*, GLShader>::iterator it = shaders.begin();
    for (; it != shaders.end(); ++it) {
        it->first->ChangedEvent().Detach(*this);
        GLuint shads[2];
        GLsizei count;
        glGetAttachedShaders(it->second.id, 2, &count, shads);
        for (GLsizei i = 0; i < count; ++i) {
            glDeleteShader(shads[i]);
        }
        glDeleteProgram(it->second.id);
    }
    shaders.clear();
}

void GLContext::Handle(Shader::ChangedEventArg arg) {
    // logger.info << "shader changed" << logger.end;
    GLuint newid;
    try {
        newid = LoadShader(arg.shader);
    }
    catch (Exception e) {
        logger.error << e.what() << " Using previously working shader." << logger.end;
        return;
    }

    GLuint oldid = shaders[arg.shader].id;

    GLuint shads[2];
    GLsizei count;
    glGetAttachedShaders(oldid, 2, &count, shads);
    for (GLsizei i = 0; i < count; ++i) {
        glDeleteShader(shads[i]);
    }
    glDeleteProgram(oldid);
    shaders[arg.shader].id = newid;
}

void GLContext::Handle(Texture2DChangedEventArg arg) {
    ITexture2D* texr = arg.resource.get();
    //reload texture
    GLuint texid = LookupTexture(texr);
    glBindTexture(GL_TEXTURE_2D, texid);
    CHECK_FOR_GL_ERROR();

    // Setup texture parameters
    SetupTexParameters(texr);

    GLenum colorFormat = GLColorFormat(texr->GetColorFormat());

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    arg.xOffset,
                    arg.yOffset,
                    texr->GetWidth(),
                    texr->GetHeight(),
                    colorFormat,
                    texr->GetType(),
                    texr->GetVoidDataPtr());
    CHECK_FOR_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);

}

void GLContext::Handle(IDataBlockChangedEventArg arg) {    
    IDataBlock* bo = arg.resource.get();
    GLuint id = bo->GetID();
    
    glBindBuffer(bo->GetBlockType(), id);
    CHECK_FOR_GL_ERROR();
        
    unsigned int size = GLTypeSize(bo->GetType()) * bo->GetSize() * bo->GetDimension();
    GLenum access = GLAccessType(bo->GetBlockType(), bo->GetUpdateMode());
    glBufferData(bo->GetBlockType(), 
                 size,
                 bo->GetVoidDataPtr(), access);
    glBindBuffer(bo->GetBlockType(), 0);
    
    if (bo->GetUnloadPolicy() == UNLOAD_AUTOMATIC)
        bo->Unload();
}


} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

