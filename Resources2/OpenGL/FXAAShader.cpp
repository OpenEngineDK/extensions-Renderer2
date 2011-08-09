// GLSL FXAA shader implementation 
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources2/OpenGL/FXAAShader.h>

#include <Renderers2/OpenGL/GLContext.h>
#include <Resources/DirectoryManager.h>
#include <Resources/File.h>
#include <Resources/DataBlock.h>
#include <Display2/Canvas3D.h>
#include <Logging/Logger.h>

namespace OpenEngine {
namespace Resources2 {
namespace OpenGL {

using Resources::File;
using Resources::DirectoryManager;

using namespace Renderers2::OpenGL;
using namespace Resources;

FXAAShader::FXAAShader()
    : active(true)
    , rcpFrame(GetUniform("rcpFrame"))
    , texA(GetTexture2D("texA"))
{
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
    if (arg.renderer.GetCurrentStage() == GLRenderer::RENDERER_INITIALIZE) {
        const float verts[4*2] = {
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f
        };
        DataBlock<2,float>* db = new DataBlock<2,float>(4);
        memcpy(db->GetVoidDataPtr(), verts, 4 * 2 * sizeof(float));
        GetAttribute("inA").Set(IDataBlockPtr(db));        
        return;
    }
    if (!active) return;
    

    // this is a hack! Module should not be added in the first place if shader is not supported.
    if (!arg.renderer.GetContext()->ShaderSupport()) return; 

    GLContext* ctx = arg.renderer.GetContext();

    GLContext::Attachments& atts = ctx->LookupCanvas(arg.canvas);
    texA.Set(atts.color0);
    rcpFrame.Set(Vector<2,float>(1.0f / arg.canvas->GetWidth(), 1.0f / arg.canvas->GetHeight()));    
    
    GLint prevFbo; 
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    GLint fbo = ctx->LookupFBO(arg.canvas);
    
    if (prevFbo == fbo) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                               ctx->LookupTexture(atts.color1.get()), 0);
        //flip output buffers.
        ITexture2DPtr tmp = atts.color0;
        atts.color0 = atts.color1;
        atts.color1 = tmp;
    }

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    //draw quad
    arg.renderer.Apply(this);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_FOR_GL_ERROR();
    arg.renderer.Release(this);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
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
