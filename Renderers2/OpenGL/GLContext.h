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
namespace Renderers2 {
namespace OpenGL {

using Resources::ITexture2D;
using Resources::IDataBlock;
using Resources::BlockType;
using Resources::Types::Type;
using Resources::UpdateMode;
using Resources::ColorFormat;
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
    map<ITexture2D*, GLint> textures;
    map<IDataBlock*, GLint> vbos;

    GLint LoadTexture(ITexture2D* tex);
    GLint LoadVBO(IDataBlock* db);
    void SetupTexParameters(ITexture2D* tex);
public:
    GLContext();
    virtual ~GLContext();

    void Init();

    bool FBOSupport();
    bool VBOSupport();
    bool ShaderSupport();
    
    GLint LookupTexture(ITexture2D* tex);
    GLint LookupVBO(IDataBlock* db);
    
    static inline GLint GLInternalColorFormat(ColorFormat f);
    static inline GLenum GLColorFormat(ColorFormat f);
    static inline GLenum GLAccessType(BlockType b, UpdateMode u);
    static inline unsigned int GLTypeSize(Type t);

};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _OE_OPENGL_CONTEXT_H_
