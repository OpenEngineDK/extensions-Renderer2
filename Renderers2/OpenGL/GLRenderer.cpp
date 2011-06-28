// OpenGL renderer implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers2/OpenGL/GLRenderer.h>

#include <Renderers2/OpenGL/RenderingView.h>
#include <Renderers2/OpenGL/LightVisitor.h>

#include <Renderers2/OpenGL/GLContext.h>
#include <Display2/Canvas3D.h>
#include <Display/IViewingVolume.h>
#include <Meta/OpenGL.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using Display::IViewingVolume;

using namespace Math;

GLRenderer::GLRenderer(GLContext* ctx)
    : ctx(ctx)
    , canvas(NULL)
    , init(false)
    , rv(new RenderingView())
    , lv(new LightVisitor())
{
    preProcess.Attach(*lv);
    process.Attach(*rv);
}
    
GLRenderer::~GLRenderer() {

}

void GLRenderer::RenderScene(Canvas3D* canvas, Time start, unsigned int approx) {

    // @todo: assert we are in preprocess stage
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the screen and the depth buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    IViewingVolume* volume = canvas->GetViewingVolume();
    // If no viewing volume is set for the viewport ignore it.
    if (volume != NULL) {
        volume->SignalRendering(approx);

        // Set viewport size 
        Vector<4,int> d(0, 0, canvas->GetWidth(), canvas->GetHeight());
        glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
        CHECK_FOR_GL_ERROR();

        // apply the volume
        ApplyViewingVolume(*volume);
    }
    CHECK_FOR_GL_ERROR();

    // run the processing phases
    RenderingEventArg rarg(*canvas, *this, start, approx);
    this->preProcess.Notify(rarg);
    // this->stage = RENDERER_PROCESS;
    this->process.Notify(rarg);
    // this->stage = RENDERER_POSTPROCESS;
    this->postProcess.Notify(rarg);
    // this->stage = RENDERER_PREPROCESS;
}


void GLRenderer::Handle(Core::InitializeEventArg arg) {
    ctx->Init();
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);						   
    CHECK_FOR_GL_ERROR();
    // Set perspective calculations to most accurate
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    CHECK_FOR_GL_ERROR();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    CHECK_FOR_GL_ERROR();

    // this->stage = RENDERER_INITIALIZE;
    this->initialize.Notify(RenderingEventArg(*canvas, *this));
    //this->stage = RENDERER_PREPROCESS;
    CHECK_FOR_GL_ERROR();
    init = true;
}
    
void GLRenderer::Handle(Core::DeinitializeEventArg arg) {

}
    
void GLRenderer::Handle(Core::ProcessEventArg arg) {
    RenderScene(canvas, arg.start, arg.approx);
}

IEvent<RenderingEventArg>& GLRenderer::InitializeEvent() {
    return initialize;
}

IEvent<RenderingEventArg>& GLRenderer::PreProcessEvent() {
    return preProcess;
}
    
IEvent<RenderingEventArg>& GLRenderer::ProcessEvent() {
    return process;
}

IEvent<RenderingEventArg>& GLRenderer::PostProcessEvent() {
    return postProcess;
}
    
IEvent<RenderingEventArg>& GLRenderer::DeinitializeEvent() {
    return deinitialize;
}

void GLRenderer::SetBackgroundColor(RGBAColor color) {
    bgc = color;
}
    
RGBAColor GLRenderer::GetBackgroundColor() {
    return bgc;
}

void GLRenderer::SetCanvas(Canvas3D* canvas) {
    this->canvas = canvas;
}

Canvas3D* GLRenderer::GetCanvas() {
    return canvas;
}


void GLRenderer::ApplyViewingVolume(IViewingVolume& volume) {
    // Select The Projection Matrix
    glMatrixMode(GL_PROJECTION);
    CHECK_FOR_GL_ERROR();

    // Reset The Projection Matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Setup OpenGL with the volumes projection matrix
    Matrix<4,4,float> projMatrix = volume.GetProjectionMatrix();
    float arr[16] = {0};
    projMatrix.ToArray(arr);
    glMultMatrixf(arr);
    CHECK_FOR_GL_ERROR();

    // Select the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    CHECK_FOR_GL_ERROR();

    // Reset the modelview matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Get the view matrix and apply it
    Matrix<4,4,float> matrix = volume.GetViewMatrix();
    float f[16] = {0};
    matrix.ToArray(f);
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
}

GLContext* GLRenderer::GetContext() {
    return ctx;
}


} // NS OpenGL
} // NS Renderers
} // NS OpenEngine
