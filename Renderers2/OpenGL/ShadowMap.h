// Shadow map rendering effect.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_SHADOW_MAP_EFFECT_H_
#define _OE_SHADOW_MAP_EFFECT_H_

#include <Core/IListener.h>
#include <Renderers2/OpenGL/GLRenderer.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Math/Vector.h>
#include <Math/Matrix.h>
#include <Meta/OpenGL.h>

namespace OpenEngine {
    namespace Scene {
        class ISceneNode;
        class TransformationNode;
        class MeshNode;
    }
    namespace Display {
        class IViewingVolume;
    }
    namespace Display2 {
        class Canvas3D;
    }
    namespace Resources2 {
        class ShaderResource;
        class Uniform;
        typedef boost::shared_ptr<ShaderResource> ShaderResourcePtr;
    }

namespace Renderers2 {
namespace OpenGL {

using Core::IListener;
using Scene::ISceneNodeVisitor;
using Scene::TransformationNode;
using Scene::ISceneNode;
using Scene::MeshNode;
using Math::Vector;
using Math::Matrix;
using Display::IViewingVolume;
using Display2::Canvas3D;
using Resources2::ShaderResourcePtr;
using Resources2::Uniform;

/**
 * Shadow Map effect. 
 * Preprocessing creates the shadow maps.
 * Postprocessing applies the maps to generate shadows in the final image.
 *
 * @class ShadowMap ShadowMap.h Renderers2/ShadowMap.h
 */
class ShadowMap: public IListener<RenderingEventArg> {
private:
    class DepthRenderer : public ISceneNodeVisitor {
    private:
        Matrix<4,4,float> modelViewMatrix, projectionMatrix;
        unsigned int width, height;
        Canvas3D* canvas;
    public:
        DepthRenderer(unsigned int width, unsigned int height);
        void Initialize(GLContext* ctx, Shader* shader);
        void Render(ISceneNode* root, IViewingVolume& cam, GLContext* ctx);
        void VisitTransformationNode(TransformationNode* node);
        void VisitMeshNode(MeshNode* node);
    };

    DepthRenderer depthRenderer;
    Display::IViewingVolume* viewingVolume;
    ShaderResourcePtr shader;
    Uniform &lightMatrix, &viewProjectionInverse;
    
public:
    ShadowMap(unsigned int width, unsigned int height); 
    void Handle(RenderingEventArg arg);
    void SetViewingVolume(IViewingVolume* v);
};

} // NS Scene
} // NS OpenEngine
}
#endif // _OE_SHADOW_MAP_EFFECT_H_
