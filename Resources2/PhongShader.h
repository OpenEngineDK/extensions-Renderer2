// GLSL Phong shader implementation
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_GLSL_PHONG_SHADER_H_
#define _OE_GLSL_PHONG_SHADER_H_

#include <Resources2/Shader.h>

namespace OpenEngine {
    namespace Geometry {
        class Mesh;
    }
namespace Resources2 {

using Geometry::Mesh;

class PhongShader: public Shader {
private:
    void AddDefine(string name);
    void AddDefine(string name, int val);
public:
    PhongShader(Mesh* mesh);
    virtual ~PhongShader();
};

}
}

#endif //_OPENGL_PHONG_SHADER_RESOURCE_H_
