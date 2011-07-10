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
#include <Renderers2/OpenGL/LightVisitor.h>
#include <Meta/OpenGL.h>
#include <Math/Matrix.h>

#include <map>

namespace OpenEngine {
    // Forward declarations.
    namespace Geometry {
        class Mesh;
        class Material;
        class GeometrySet;
    }
    namespace Resources2 {
        class Shader;
        class PhongShader;
    }

namespace Renderers2 {
namespace OpenGL {

using Scene::ISceneNodeVisitor;
using Core::IListener;
using Geometry::Mesh;
using Geometry::GeometrySet;
using Geometry::Material;
using Math::Matrix;
using Scene::RenderStateNode;
using Scene::TransformationNode;
using Scene::MeshNode;
using Resources2::PhongShader;
using Resources2::Shader;
using std::map;

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

    map<Mesh*, PhongShader*> shaders; // hack until material type is revised

    Matrix<4,4,float> modelViewMatrix, projectionMatrix;

    inline void ApplyRenderState(RenderStateNode* node);
    inline void BindUniforms(Shader* shad, GLint id);
    inline void BindAttributes(Shader* shad, GLint id);
    inline void BindTextures2D(Shader* shad, GLint id);
public:
    LightVisitor::LightSource light;
    
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
