// OpenGL renderer context
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_OPENGL_CONTEXT_H_
#define _OE_OPENGL_CONTEXT_H_

#include <Resources/ITexture2D.h>
#include <Resources/IDataBlock.h>
#include <Resources2/Shader.h>
#include <Meta/OpenGL.h>
#include <Core/IListener.h>
#include <Utils/Box.h>
#include <map>
#include <vector>
#include <set>

namespace OpenEngine {
    namespace Resources {
        class ICubemap;
    }
    namespace Display2 {
        class ICanvas;
        class Canvas2D;
        class Canvas3D;
    }
namespace Renderers2 {
namespace OpenGL {

using Resources::ITexture2D;
using Resources::ITexture2DPtr;
using Resources::Texture2DChangedEventArg;
using Resources::IDataBlockChangedEventArg;
using Resources::IDataBlock;
using Resources::IDataBlockPtr;
using Resources::BlockType;
using Resources::Types::Type;
using Resources::UpdateMode;
using Resources::ColorFormat;
using Resources::ICubemap;
using Resources::ICubemapPtr;
using Resources2::Shader;
using Resources2::Uniform;
using Display2::ICanvas;
using Display2::Canvas2D;
using Display2::Canvas3D;
using Core::IListener;
using Utils::Box;
using std::map;
using std::pair;
using std::vector;
using std::set;

/**
 * OpenGL Shader Language versions
 */
enum GLSLVersion { GLSL_UNKNOWN, GLSL_NONE, GLSL_14, GLSL_20 };

/**
 * OpenGL Context
 *
 * A map between OpenEngine data types and GL bound resources.
 *
 * @class GLContext GLContext.h Renderers2/OpenGL/GLContext.h
 */
class GLContext: public IListener<Shader::ChangedEventArg>
               , public IListener<Uniform::ChangedEventArg>
               , public IListener<Texture2DChangedEventArg> 
               , public IListener<IDataBlockChangedEventArg> {
public:
    // structure containing the uniform locations.
    struct GLShader {
        GLuint id;
        map<Uniform*, GLint> uniforms;
        vector<pair<Box<IDataBlockPtr>*, GLint> > attributes;
        vector<pair<Box<ITexture2DPtr>*, GLint> > textures;
        vector<pair<Box<ICubemapPtr>*, GLint> > cubemaps;
    };
    struct Attachments {
        ITexture2DPtr color0, color1, depth;
    };

private:
    GLSLVersion glslversion;
    bool init, fboSupport, vboSupport, shaderSupport;
    map<ICanvas*, Attachments> attachments; // color attachments and depth attachment
    map<ICanvas*, GLuint> fbos;             // association with fbo
    map<ITexture2D*, GLuint> textures;
    map<IDataBlock*, GLuint> vbos;
    map<ICubemap*, GLuint> cubemaps;
    map<Shader*, GLShader> shaders;

    map<Shader*, set<Uniform*> > uniformQueue; // queue to delay uniform updates.

    // GPU creation routines
    Attachments LoadCanvas(ICanvas* can);
    GLuint LoadTexture(ITexture2D* tex);
    GLuint LoadVBO(IDataBlock* db);
    GLuint LoadShader(Shader* shad);
    GLuint LoadCubemap(ICubemap* cube);
    inline void BindUniform(Uniform& uniform, GLint loc);
    inline GLShader ResolveLocations(GLuint id, Shader* shad);
    inline void SetupTexParameters(ITexture2D* tex);


    // inline void BindUniforms(GLContext::GLShader& glshader);
    inline void FlushUniforms(Shader* shader, GLShader& glshader);
    inline void BindAttributes(GLContext::GLShader& glshader);
    inline void UnbindAttributes(GLContext::GLShader& glshader);
    inline void BindTextures2D(GLContext::GLShader& glshader);
    inline void UnbindTextures2D(GLContext::GLShader& glshader);


public:
    GLContext();
    virtual ~GLContext();

    void Init();

    bool FBOSupport();
    bool VBOSupport();
    bool ShaderSupport();
        
    // lookup routines. If no map contains the requested object the
    // creation routines will be invoked.
    GLuint LookupFBO(ICanvas* can);
    Attachments& LookupCanvas(Canvas2D* can);
    Attachments& LookupCanvas(ICanvas* can);
    GLuint LookupTexture(ITexture2D* tex);
    GLuint LookupVBO(IDataBlock* db);
    GLShader LookupShader(Shader* shad);
    GLuint LookupCubemap(ICubemap* cube);

    // mainly for debugging and testing
    void ReleaseTextures();
    void ReleaseVBOs();
    void ReleaseShaders();
    
    // conversion from OE STUFF to GL STUFF
    static GLint GLInternalColorFormat(ColorFormat f);
    static GLenum GLColorFormat(ColorFormat f);
    static GLenum GLAccessType(BlockType b, UpdateMode u);
    static unsigned int GLTypeSize(Type t);

    void Handle(Shader::ChangedEventArg arg);
    void Handle(Uniform::ChangedEventArg arg);
    void Handle(Texture2DChangedEventArg arg);
    void Handle(IDataBlockChangedEventArg arg);

    GLuint Apply(Shader* shader);
    void Release(Shader* shader);

};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _OE_OPENGL_CONTEXT_H_
