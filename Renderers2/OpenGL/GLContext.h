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

#include <Resources/ITexture.h>
#include <Resources/IDataBlock.h>
#include <Meta/OpenGL.h>
#include <map>

namespace OpenEngine {
    namespace Resources {
        class ITexture2D;
    }
    namespace Resources2 {
        class Shader;
    }
    namespace Display2 {
        class ICanvas;
    }
namespace Renderers2 {
namespace OpenGL {

using Resources::ITexture2D;
using Resources::IDataBlock;
using Resources::BlockType;
using Resources::Types::Type;
using Resources::UpdateMode;
using Resources::ColorFormat;
using Resources2::Shader;
using Display2::ICanvas;
using std::map;

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
class GLContext {
private:
    GLSLVersion glslversion;
    bool init, fboSupport, vboSupport, shaderSupport;
    map<ICanvas*, GLuint> canvases;
    map<ITexture2D*, GLuint> textures;
    map<IDataBlock*, GLuint> vbos;
    map<Shader*, GLuint> shaders;

    GLuint LoadCanvas(ICanvas* can);
    GLuint LoadTexture(ITexture2D* tex);
    GLuint LoadVBO(IDataBlock* db);
    GLuint LoadShader(Shader* shad);

    inline void SetupTexParameters(ITexture2D* tex);
public:
    GLContext();
    virtual ~GLContext();

    void Init();

    bool FBOSupport();
    bool VBOSupport();
    bool ShaderSupport();
    
    GLuint LookupCanvas(ICanvas* can);
    GLuint LookupTexture(ITexture2D* tex);
    GLuint LookupVBO(IDataBlock* db);
    GLuint LookupShader(Shader* shad);

    // mainly for debugging and testing
    void ReleaseTextures();
    void ReleaseVBOs();
    void ReleaseShaders();
    
    static GLint GLInternalColorFormat(ColorFormat f);
    static GLenum GLColorFormat(ColorFormat f);
    static GLenum GLAccessType(BlockType b, UpdateMode u);
    static unsigned int GLTypeSize(Type t);

};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _OE_OPENGL_CONTEXT_H_
