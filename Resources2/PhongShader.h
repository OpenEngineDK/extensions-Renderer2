// GLSL Phong shader implementation
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_GLSL_PHONG_SHADER_H_
#define _OE_GLSL_PHONG_SHADER_H_

#include <Resources2/ShaderResource.h>
#include <Renderers2/OpenGL/LightVisitor.h>
#include <Geometry/Material.h>

namespace OpenEngine {
    namespace Geometry {
        class Mesh;
    }
namespace Resources2 {

using Geometry::Mesh;
using Geometry::Material;
using Renderers2::OpenGL::LightVisitor;

class PhongShader: public ShaderResource
                 , public IListener<Material::ChangedEventArg> {
private:
    void AddDefine(string name);
    void AddDefine(string name, int val);
    string defines;

    void UpdateMaterial(Material* mat);

public:
    PhongShader(Mesh* mesh);
    virtual ~PhongShader();

    void SetModelViewMatrix(Matrix<4,4,float> m);
    void SetModelViewProjectionMatrix(Matrix<4,4,float> m);

    void SetLight(LightVisitor::LightSource l, Vector<4,float> globalAmbient);

    string GetVertexShader();
    string GetFragmentShader();

    void Handle(Material::ChangedEventArg arg);
};

}
}

#endif //_OPENGL_PHONG_SHADER_RESOURCE_H_
