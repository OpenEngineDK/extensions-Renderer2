// GLSL FXAA shader implementation 
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources2/OpenGL/FXAAShader.h>

#include <Renderers2/OpenGL/GLContext.h>
#include <Resources/DirectoryManager.h>
#include <Resources/File.h>
#include <Display2/Canvas3D.h>
#include <Logging/Logger.h>

namespace OpenEngine {
namespace Resources2 {
namespace OpenGL {

using Resources::File;
using Resources::DirectoryManager;

using namespace Renderers2::OpenGL;

FXAAShader::FXAAShader()
    : active(true)
{

    vertices[0] = -1.0;
    vertices[1] = 1.0;

    vertices[2] = -1.0;
    vertices[3] = -1.0;

    vertices[4] = 1.0;
    vertices[5] = -1.0;

    vertices[6] = 1.0;
    vertices[7] = 1.0;

    const string shaderFile = DirectoryManager::FindFileInPath("extensions/Renderer2/shaders/TinyFxaa.glsl");

    int sz = File::GetSize(shaderFile);
    char* buf = new char[sz];
    ifstream* f = File::Open(shaderFile);
    f->read(buf, sz-1);
    buf[sz-1] = '\0';
    f->close();
    string shader = string(buf, sz);
    delete buf;
    delete f;

    string vdef = "#define PRG_2_V\n";
    string fdef = "#define PRG_2_F\n";

    vertexShader = vdef + shader;
    fragmentShader = fdef + shader;
}

FXAAShader::~FXAAShader() {

}

void FXAAShader::Handle(RenderingEventArg arg) {
    if (!active) return;

    // this is a hack! Module should not be added in the first place if shader is not supported.
    if (!arg.renderer.GetContext()->ShaderSupport()) return; 

    GLint shaderId = arg.renderer.GetContext()->LookupShader(this);
    GLint screenId = arg.renderer.GetContext()->LookupCanvas(arg.canvas);

    glDisable(GL_DEPTH_TEST);

    glUseProgram(shaderId);
    GLint loc = glGetUniformLocation(shaderId, "rcpFrame");
#if OE_SAFE
    if (loc == -1) throw Exception(string("Uniform location not found: rcpFrame"));
#endif
    glUniform2f(loc, (1.0f / arg.canvas->GetWidth())*1.0f, (1.0f / arg.canvas->GetHeight()) * 1.0f);    

    loc = glGetUniformLocation(shaderId, "texA");
#if OE_SAFE
    if (loc == -1) throw Exception(string("Uniform location not found: texA"));
#endif
    glActiveTexture(GL_TEXTURE0);
    CHECK_FOR_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, screenId);
    CHECK_FOR_GL_ERROR();
    glUniform1i(loc, 0);
    CHECK_FOR_GL_ERROR();
    
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, arg.canvas->GetWidth(), arg.canvas->GetHeight(), 0);
    CHECK_FOR_GL_ERROR();
    
    //draw quad
    loc = glGetAttribLocation(shaderId, "inA");
#if OE_SAFE
    if (loc == -1) throw Exception(string("Attribute location not found: inA"));
#endif
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, 0, 0, vertices);
    CHECK_FOR_GL_ERROR();

    glDrawArrays(GL_QUADS, 0, 4);
    CHECK_FOR_GL_ERROR();

    glDisableVertexAttribArray(loc);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    glEnable(GL_DEPTH_TEST);
}

void FXAAShader::SetActive(bool active) {
    this->active = active;
}

bool FXAAShader::GetActive() {
    return active;
}

}
}
}
