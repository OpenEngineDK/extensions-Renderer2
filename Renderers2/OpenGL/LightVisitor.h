// OpenGL light preprocessor implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_LIGHT_VISITOR_H_
#define _OE_LIGHT_VISITOR_H_

#include <Renderers2/OpenGL/GLRenderer.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Core/IListener.h>
#include <Meta/OpenGL.h>
#include <Math/Matrix.h>
#include <Math/Vector.h>
#include <vector>

namespace OpenEngine {

    //forward declarations
    namespace Scene {
        class TransformationNode;
        class PointLightNode;
        class DirectionalLightNode;
        class SpotLightNode;
    }
    
namespace Renderers2 {
namespace OpenGL {

using OpenEngine::Scene::TransformationNode;
using OpenEngine::Scene::PointLightNode;
using OpenEngine::Scene::DirectionalLightNode;
using OpenEngine::Scene::SpotLightNode;
using OpenEngine::Scene::ISceneNodeVisitor;
using OpenEngine::Core::IListener;
using Math::Matrix;
using Math::Vector;

using std::vector;
/**
 * Setup OpenGL lighting
 *
 * @class LightVisitor LightVisitor.h Renderers2/OpenGL/LightVisitor.h
 */
class LightVisitor: public ISceneNodeVisitor, public IListener<RenderingEventArg> {
public:
    struct LightSource {
        Vector<4,float> position;
        float constantAttenuation;
        float linearAttenuation;
        float quadraticAttenuation;
        Vector<4,float> ambient;
        Vector<4,float> diffuse;
        Vector<4,float> specular;
        
        Vector<3,float> spotDirection;
        float spotCutoff;
        float spotExponent;
    };
private:
    float pos[4], dir[4];
    GLint count;
    Matrix<4,4,float> modelViewMatrix;
    vector<LightSource> lights;
public:
    LightVisitor(); 
    ~LightVisitor();
        
    void Handle(RenderingEventArg arg);
    void VisitTransformationNode(TransformationNode* node);
    void VisitDirectionalLightNode(DirectionalLightNode* node);
    void VisitPointLightNode(PointLightNode* node);
    void VisitSpotLightNode(SpotLightNode* node);
    
    vector<LightSource> GetLights();
};

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers

#endif
