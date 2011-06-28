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

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using namespace Resources;

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
        return 0;
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


GLint GLContext::LoadTexture(ITexture2D* tex) {
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

GLint GLContext::LookupTexture(ITexture2D* tex) {
    map<ITexture2D*, GLint>::iterator it = textures.find(tex);
    if (it != textures.end())
        return (*it).second;
    GLint id = LoadTexture(tex);
    textures[tex] = id;
    return id;
}

GLint GLContext::LoadVBO(IDataBlock* db) {
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

GLint GLContext::LookupVBO(IDataBlock* db) {
    map<IDataBlock*, GLint>::iterator it = vbos.find(db);
    if (it != vbos.end())
        return (*it).second;
    GLint id = LoadVBO(db);
    vbos[db] = id;
    return id;
}


} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

