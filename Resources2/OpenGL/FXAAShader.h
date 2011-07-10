// GLSL FXAA shader implementation 
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_GLSL_FXAA_SHADER_H_
#define _OE_GLSL_FXAA_SHADER_H_

#include <Resources2/Shader.h>
#include <Renderers2/OpenGL/GLRenderer.h>

namespace OpenEngine {
namespace Resources2 {
namespace OpenGL {

using Core::IListener;
using Renderers2::OpenGL::RenderingEventArg;

class FXAAShader: public Shader, public IListener<RenderingEventArg> {
private:
    float vertices[4*2]; // 4 2-vectors
    bool active;
public:
    FXAAShader();
    virtual ~FXAAShader();

    void Handle(RenderingEventArg arg);

    void SetActive(bool active);
    bool GetActive();
};

}
}
}

#endif //_OE_GLSL_FXAA_SHADER_H_
