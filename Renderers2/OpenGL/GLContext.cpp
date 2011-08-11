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
#include <Display2/Canvas3D.h>

#include <Resources/ITexture2D.h>
#include <Resources/Texture2D.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using namespace Resources;
using Resources2::Uniform;
using Resources2::Shader;
using Display2::ICanvas;

using Resources::ITexture2DPtr;
using Resources::Texture2D;

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
#ifdef OE_IOS
    fboSupport = true;
    vboSupport = true;
    shaderSupport = true;
#else
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
#endif
    
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
#ifndef OE_IOS
    case ALPHA_COMPRESSED: return GL_COMPRESSED_ALPHA;
    case LUMINANCE_COMPRESSED: return GL_COMPRESSED_LUMINANCE;
    case LUMINANCE32F: return GL_R32F;
    case LUMINANCE_ALPHA_COMPRESSED: return GL_COMPRESSED_LUMINANCE_ALPHA;
    case RGB_COMPRESSED: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case RGBA_COMPRESSED: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case RGB32F: return GL_RGB32F;
    case RGBA32F: return GL_RGBA32F;
#endif
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
#ifndef OE_IOS
    case BGR: 
        return GL_BGR;
    case BGRA: 
        return GL_BGRA;
#endif
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
#ifndef OE_IOS
    case Types::DOUBLE:
        return sizeof(GLdouble);
#endif
    default:
        //case Types::NOTYPE:
        throw Exception("GLTypeSize: Unknown type.");
    }
    return sizeof(GLshort);
}

GLenum GLContext::GLAccessType(BlockType b, UpdateMode u){
    if (u == STATIC){
        switch (b){
#ifndef OE_IOS
        case PIXEL_PACK:
            return GL_STATIC_COPY;
#endif
        default:
            return GL_STATIC_DRAW;
        }
    }else if (u == DYNAMIC){
        switch (b){
#ifndef OE_IOS
        case PIXEL_PACK:
            return GL_DYNAMIC_COPY;
#endif
        default:
            return GL_DYNAMIC_DRAW;
        }
    }
    return GL_STATIC_DRAW;
}



// ------- Canvas -------
GLuint GLContext::LookupFBO(ICanvas* can) {
    map<ICanvas*, GLuint>::iterator it = fbos.find(can);
    if (it != fbos.end())
        return it->second;
    
    GLuint fbo;
    glGenFramebuffers(1, &fbo);

    fbos[can] = fbo;
    return fbo;
}

GLContext::Attachments GLContext::LoadCanvas(ICanvas* can) {
#if OE_SAFE
    if (can == NULL) throw Exception("Cannot load NULL canvas.");
#endif
    GLContext::Attachments atts;

    atts.color0 = ITexture2DPtr(new Texture2D<unsigned char>(can->GetWidth(), can->GetHeight(), can->GetColorFormat(), 3));
    atts.color1 = ITexture2DPtr(new Texture2D<unsigned char>(can->GetWidth(), can->GetHeight(), can->GetColorFormat(), 3));
    atts.depth = ITexture2DPtr(new Texture2D<float>(can->GetWidth(), can->GetHeight(), DEPTH, 1));
    return atts;
}

GLContext::Attachments& GLContext::LookupCanvas(Canvas2D* can) {
    GLContext::Attachments& atts = LookupCanvas((ICanvas*)can);
    atts.color0 = can->GetTexture();
    return atts;
}

GLContext::Attachments& GLContext::LookupCanvas(ICanvas* can) {
    map<ICanvas*, Attachments>::iterator it = attachments.find(can);
    if (it != attachments.end())
        return it->second;

    GLContext::Attachments atts = LoadCanvas(can);
    attachments[can] = atts;
    
    return attachments[can];
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
#ifndef OE_IOS
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
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

#ifdef OE_IOS
    // es test
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return;
#endif
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex->GetWrapping());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex->GetWrapping());

    if (tex->UseMipmapping()) {
#ifndef OE_IOS
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->GetFiltering());
#endif
    } 
    else {
#ifndef OE_IOS
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
#endif
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

#ifdef OE_IOS
    string iosHeader = string("precision mediump float;\n");
#endif

    // compile vertex shader
    const GLchar* shaderBits[1];
    string vertexShader = shad->GetVertexShader();
#ifdef OE_IOS
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
#ifdef OE_IOS
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

void GLContext::BindUniform(Uniform& uniform, GLint loc) {
    const Uniform::Data data = uniform.GetData();
    switch (uniform.GetKind()) {
    case Uniform::INT:
        glUniform1i(loc, data.i);
        break;
    case Uniform::FLOAT:
        glUniform1f(loc, data.f);
        break;
    case Uniform::FLOAT2:
        glUniform2fv(loc, 1, data.fv);
        break;
    case Uniform::FLOAT3:
        glUniform3fv(loc, 1, data.fv);
        break;
    case Uniform::FLOAT4:
        glUniform4fv(loc, 1, data.fv);
        break;
    case Uniform::MAT3X3:
        glUniformMatrix3fv(loc, 1, false, data.fv);
        break;
    case Uniform::MAT4X4:
        glUniformMatrix4fv(loc, 1, false, data.fv);
        break;            
    case Uniform::UNKNOWN:
#if OE_SAFE
        throw Exception("Unknown uniform kind.");
#endif
        break;
    }
    CHECK_FOR_GL_ERROR();
}

GLContext::GLShader GLContext::ResolveLocations(GLuint id, Shader* shad) {
    GLContext::GLShader glshader;
    glshader.id = id;
    
    GLint count, maxLength;
    glGetProgramiv(id,
                   GL_ACTIVE_UNIFORMS,
                   &count);
    glGetProgramiv(id,
                   GL_ACTIVE_UNIFORM_MAX_LENGTH,
                   &maxLength);

    char* name = new char[maxLength+1];
    GLint out, size;
    GLenum type;
    for (int i = 0; i < count; ++i) {
        glGetActiveUniform(id,
                           i,
                           maxLength,
                           &out,
                           &size,
                           &type,
                           name);
        // logger.info << "type: " << type << logger.end;
        // logger.info << "unif: " << string(name) << logger.end;

        GLint loc = glGetUniformLocation(id, name);
        if (loc == -1) continue;

        switch(type) {
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_SHADOW:
            // logger.info << "sampler2d(shadow)" << logger.end;
            glshader.textures.push_back(make_pair(&shad->GetTexture2D(string(name)), loc));
            break;
        case GL_SAMPLER_CUBE:
                // logger.info << "samplercube" << logger.end;
                glshader.cubemaps.push_back(make_pair(&shad->GetCubemap(string(name)), loc));
                break;
        default:
            glshader.uniforms[&shad->GetUniform(string(name))] = loc;
        }
    } 
    delete[] name; 

    glUseProgram(glshader.id);
    // bind the uniforms set at resolve time
    for (map<Uniform*, GLint>::iterator it = glshader.uniforms.begin();
         it != glshader.uniforms.end(); ++it) {
        Uniform& uniform = *it->first;
        if (uniform.GetKind() != Uniform::UNKNOWN)
            BindUniform(uniform, it->second);
    }

    // set the texture unit locations
    GLuint texUnit = 0;
    for (; texUnit < glshader.textures.size(); ++texUnit) {
        GLint loc = glshader.textures[texUnit].second;
        glUniform1i(loc, texUnit);
        CHECK_FOR_GL_ERROR();
    }
    for (unsigned int i = 0; i < glshader.cubemaps.size(); ++i) {
        ++texUnit;
        GLint loc = glshader.cubemaps[i].second;
        glUniform1i(loc, texUnit);
        CHECK_FOR_GL_ERROR();
    }
    glUseProgram(0);

    // Attributes
    glGetProgramiv(id,
                   GL_ACTIVE_ATTRIBUTES,
                   &count);
    glGetProgramiv(id,
                   GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,
                   &maxLength);
    name = new char[maxLength+1];

    for (int i = 0; i < count; ++i) {
        glGetActiveAttrib(id,
                          i,
                          maxLength,
                          &out,
                          &size,
                          &type,
                          name);
        GLint loc = glGetAttribLocation(id, name);
        if (loc == -1) continue;
        glshader.attributes.push_back(make_pair(&shad->GetAttribute(string(name)), loc)); 
    } 
    delete[] name; 

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
    shad->UniformChangedEvent().Attach(*this);
    return glshader;
}

void GLContext::ReleaseTextures() {
    map<ITexture2D*, GLuint>::iterator it = textures.begin();
    for (; it != textures.end(); ++it) {
        glDeleteTextures(1, &it->second);
        it->first->ChangedEvent().Detach(*this);
    }
    
    map<ICubemap*, GLuint>::iterator it4 = cubemaps.begin();
    for (; it4 != cubemaps.end(); ++it4) {
        glDeleteTextures(1, &it4->second);
    }
    
    // also release fbos since attachments where released
    map<ICanvas*, GLuint>::iterator it5 = fbos.begin();
    for (; it5 != fbos.end(); ++it5) {
        glDeleteFramebuffers(1, &it5->second);
    }
    
    textures.clear();
    cubemaps.clear();
    fbos.clear();
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
        it->first->UniformChangedEvent().Detach(*this);
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
    shaders[arg.shader] = ResolveLocations(newid, arg.shader);
}

void GLContext::Handle(Uniform::ChangedEventArg arg) {
    // logger.info << "changed" << logger.end;
    const GLContext::GLShader glshader = LookupShader(arg.shader);
    map<Uniform*, GLint>::const_iterator it = glshader.uniforms.find(arg.uniform);
    if (it == glshader.uniforms.end())
        return;
    // just rebind immediately
    // todo: queue this operation
    glUseProgram(glshader.id);
    BindUniform(*arg.uniform, it->second);
    glUseProgram(0);
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

