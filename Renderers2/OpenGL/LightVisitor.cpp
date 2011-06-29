// OpenGL light preprocessor implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers2/OpenGL/LightVisitor.h>
#include <Display2/Canvas3D.h>
#include <Scene/TransformationNode.h>
#include <Scene/DirectionalLightNode.h>
#include <Scene/PointLightNode.h>
#include <Scene/SpotLightNode.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;

LightVisitor::LightVisitor()
    : count(0)
{
    pos[0] = 0.0;
    pos[1] = 0.0;
    pos[2] = 0.0;
    pos[3] = 1.0;
    dir[0] = 0.0;
    dir[1] = -1.0;
    dir[2] = 0.0;
    dir[3] = 0.0;
}

LightVisitor::~LightVisitor() {}
        
void LightVisitor::VisitTransformationNode(TransformationNode* node) {
    Matrix<4,4,float> m = node->GetTransformationMatrix();
    float f[16];
    m.ToArray(f);
    glPushMatrix();
    glMultMatrixf(f);
    node->VisitSubNodes(*this);
    glPopMatrix();
    CHECK_FOR_GL_ERROR();
}
    
void LightVisitor::VisitDirectionalLightNode(DirectionalLightNode* node) {
#if OE_SAFE
    GLint max;
    glGetIntegerv(GL_MAX_LIGHTS, &max);
    if (count >= max) 
        throw new Exception("OpenGL max lights exceeded.");
#endif
    GLint light = GL_LIGHT0+count;
    float color[4];
    glLightfv(light, GL_POSITION, dir);
    node->ambient.ToArray(color);
    glLightfv(light, GL_AMBIENT, color);
    node->diffuse.ToArray(color);
    glLightfv(light, GL_DIFFUSE, color);
    node->specular.ToArray(color);
    glLightfv(light, GL_SPECULAR, color);
    glEnable(light);
    count++;
    CHECK_FOR_GL_ERROR();
    node->VisitSubNodes(*this);            
}
    
void LightVisitor::VisitPointLightNode(PointLightNode* node) {
#if OE_SAFE
    GLint max;
    glGetIntegerv(GL_MAX_LIGHTS, &max);
    if (count >= max) 
        throw new Exception("OpenGL max lights exceeded.");
#endif
    GLint light = GL_LIGHT0 + count;
    float color[4];
    glLightfv(light, GL_POSITION, pos);
    node->ambient.ToArray(color);
    glLightfv(light, GL_AMBIENT, color);
    node->diffuse.ToArray(color);
    glLightfv(light, GL_DIFFUSE, color);
    node->specular.ToArray(color);
    glLightfv(light, GL_SPECULAR, color);
    glLightf(light, GL_CONSTANT_ATTENUATION, node->constAtt);
    glLightf(light, GL_LINEAR_ATTENUATION, node->linearAtt);
    glLightf(light, GL_QUADRATIC_ATTENUATION, node->quadAtt);
    glEnable(light);
    ++count;
    CHECK_FOR_GL_ERROR();
    node->VisitSubNodes(*this);
}

void LightVisitor::VisitSpotLightNode(SpotLightNode* node) {
#if OE_SAFE
    GLint max;
    glGetIntegerv(GL_MAX_LIGHTS, &max);
    if (count >= max) 
        throw new Exception("OpenGL max lights exceeded.");
#endif
    GLint light = GL_LIGHT0+count;
    float color[4];
    glLightfv(light, GL_POSITION, pos);
    glLightfv(light, GL_SPOT_DIRECTION, dir);
    glLightf(light, GL_SPOT_CUTOFF, node->cutoff);            
    glLightf(light, GL_SPOT_EXPONENT, node->exponent);            
    node->ambient.ToArray(color);
    glLightfv(light, GL_AMBIENT, color);
    node->diffuse.ToArray(color);
    glLightfv(light, GL_DIFFUSE, color);
    node->specular.ToArray(color);
    glLightfv(light, GL_SPECULAR, color);
    glLightf(light, GL_CONSTANT_ATTENUATION, node->constAtt);
    glLightf(light, GL_LINEAR_ATTENUATION, node->linearAtt);
    glLightf(light, GL_QUADRATIC_ATTENUATION, node->quadAtt);
    glEnable(light);
    ++count;
    CHECK_FOR_GL_ERROR();
    node->VisitSubNodes(*this);            
}

void LightVisitor::Handle(RenderingEventArg arg) {
    #if OE_SAFE
    if (arg.canvas->GetScene() == NULL)
        throw new Exception("Scene was NULL in LightVisitor.");
    #endif
    count = 0;
    glMatrixMode(GL_MODELVIEW);
    arg.canvas->GetScene()->Accept(*this);
    GLint max;
    glGetIntegerv(GL_MAX_LIGHTS, &max);
    for (int i = count; i < max; ++i) {
        glDisable(GL_LIGHT0 + i);
        CHECK_FOR_GL_ERROR();
    }
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
