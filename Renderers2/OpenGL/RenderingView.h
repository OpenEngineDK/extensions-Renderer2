// OpenGL rendering view.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_OPENGL_RENDERING_VIEW_H_
#define _OE_OPENGL_RENDERING_VIEW_H_

#include <Scene/ISceneNodeVisitor.h>
#include <Core/IListener.h>
#include <Renderers2/OpenGL/GLRenderer.h>

namespace OpenEngine {
    // Forward declarations.
    namespace Geometry {
        class Mesh;
        class Material;
    }

namespace Renderers2 {
namespace OpenGL {

using Scene::ISceneNodeVisitor;
using Core::IListener;
using Geometry::Mesh;
using Geometry::Material;
using Scene::RenderStateNode;
using Scene::TransformationNode;
using Scene::MeshNode;


/**
 * Concrete scene traverser and rendering tool using OpenGL.
 */
class RenderingView 
    : public ISceneNodeVisitor, public IListener<RenderingEventArg> {
private:
    GLContext* ctx;
    RenderStateNode* currentRenderState;    
    // bool renderBinormal, renderTangent, renderSoftNormal, renderHardNormal;
    bool renderTexture, renderShader;
    inline void ApplyRenderState(RenderStateNode* node);
public:
    RenderingView();
    virtual ~RenderingView();
    void VisitMeshNode(MeshNode* node);
    void VisitTransformationNode(TransformationNode* node);
    void VisitRenderStateNode(RenderStateNode* node);
    void Handle(RenderingEventArg arg);
};

} // NS OpenGL
} // NS Renderers2
} // NS OpenEngine

#endif // _OE_OPENGL_RENDERING_VIEW_H_
