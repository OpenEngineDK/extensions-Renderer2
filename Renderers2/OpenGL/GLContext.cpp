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

#include <Resources2/Shader.h>
#include <Display2/ICanvas.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using namespace Resources;
using Resources2::Uniform;
using Resources2::Shader;
using Display2::ICanvas;

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 can->GetWidth(), can->GetHeight(), 0, GL_RGB, 
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
        
    if (db->GetUnloadPolicy() == UNLOAD_AUTOMATIC)
        db->Unload();
    return id;
}

GLuint GLContext::LookupVBO(IDataBlock* db) {
    map<IDataBlock*, GLuint>::iterator it = vbos.find(db);
    if (it != vbos.end())
        return (*it).second;
    GLuint id = LoadVBO(db);
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

    // compile vertex shader
    const GLchar* shaderBits[1];
    shaderBits[0] = shad->GetVertexShader().c_str();
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
        logger.error << "compile errors: " << buffer << logger.end;
        throw Exception("Failed to compile vertex shader.");
    }
#endif
#if DEBUG
    PrintShaderInfoLog(vertexId);
#endif

    // compile fragment shader
    shaderBits[0] = shad->GetFragmentShader().c_str();
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
#if DEBUG
    PrintShaderInfoLog(fragmentId);
#endif

    // Link the program object and print out the info log
    glLinkProgram(shaderId);
    GLint linked;
    glGetProgramiv(shaderId, GL_LINK_STATUS, &linked);
#if DEBUG
    PrintProgramInfoLog(shaderId);
#endif
    CHECK_FOR_GL_ERROR();
#if OE_SAFE            
    if(linked == GL_FALSE)
        throw Exception("Failed to link shader program");
#endif
    return shaderId;
}

GLuint GLContext::LookupShader(Shader* shad) {
    map<Shader*, GLuint>::iterator it = shaders.find(shad);
    if (it != shaders.end())
        return (*it).second;
    GLuint id = LoadShader(shad);
    shaders[shad] = id;
    
    return id;
}

void GLContext::ReleaseTextures() {
    map<ITexture2D*, GLuint>::iterator it = textures.begin();
     for (; it != textures.end(); ++it) {
         glDeleteTextures(1, &it->second);
     }

     map<ICanvas*, GLuint>::iterator it2 = canvases.begin();
     for (; it2 != canvases.end(); ++it2) {
         glDeleteTextures(1, &it2->second);
     }

     textures.clear();
     canvases.clear();
}

void GLContext::ReleaseVBOs() {
    map<IDataBlock*, GLuint>::iterator it = vbos.begin();
    for (; it != vbos.end(); ++it) {
        glDeleteBuffers(1, &it->second);
    }
    vbos.clear();
}

void GLContext::ReleaseShaders() {
    map<Shader*, GLuint>::iterator it = shaders.begin();
    for (; it != shaders.end(); ++it) {
        GLuint shads[2];
        GLsizei count;
        glGetAttachedShaders(it->second, 2, &count, shads);
        for (GLsizei i = 0; i < count; ++i) {
            glDeleteShader(shads[i]);
        }
        glDeleteProgram(it->second);
    }
    shaders.clear();
}

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

