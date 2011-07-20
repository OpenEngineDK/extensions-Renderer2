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
#include <Renderers2/OpenGL/CanvasVisitor.h>

#include <Renderers2/OpenGL/GLContext.h>
#include <Display2/Canvas3D.h>
#include <Display2/CompositeCanvas.h>
#include <Display/IViewingVolume.h>
#include <Meta/OpenGL.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using Display::IViewingVolume;

using namespace Display2;
using namespace Math;

GLRenderer::GLRenderer(GLContext* ctx)
    : ctx(ctx)
    , canvas(NULL)
    , init(false)
    , rv(new RenderingView())
    , lv(new LightVisitor())
    , cv(new CanvasVisitor(*this))
    , arg(Core::ProcessEventArg(Time(), 0))
{
    preProcess.Attach(*lv);
    process.Attach(*rv);
}
    
GLRenderer::~GLRenderer() {

}

void GLRenderer::Render(CompositeCanvas* canvas) {
    // logger.info << "render composite: " << canvas << logger.end;
    canvas->AcceptChildren(*cv);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, canvas->GetWidth(), 0.0, canvas->GetHeight(), 0.0, 1.0);
    CHECK_FOR_GL_ERROR();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, canvas->GetWidth(), canvas->GetHeight());
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    // glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    const float texc[8] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glClientActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texc);
    CHECK_FOR_GL_ERROR();

    CompositeCanvas::ContainerIterator it = canvas->CanvasesBegin();
    for (; it != canvas->CanvasesEnd(); ++it) {        
        const float w = it->w;
        const float h = it->h;
        const float x = it->x;
        const float y = canvas->GetHeight() - it->y;
        const float vert[8] = {
            x, y, 
            x, y - h, 
            x + w, y - h, 
            x + w, y
        };
        glVertexPointer(2, GL_FLOAT, 0, vert);

        float col[16];
        it->color.ToArray(col);
        it->color.ToArray(&col[4]);
        it->color.ToArray(&col[8]);
        it->color.ToArray(&col[12]);
        col[3] = col[7] = col[11] = col[15] =  it->opacity;
        glColorPointer(4, GL_FLOAT, 0, col);
        // glBlendColor(col[0], col[1], col[2], col[3]);

        glBindTexture(GL_TEXTURE_2D, ctx->LookupCanvas(it->canvas));
        glDrawArrays(GL_QUADS, 0, 4);
        CHECK_FOR_GL_ERROR();
    }

    glBindTexture(GL_TEXTURE_2D, ctx->LookupCanvas(canvas));
    CHECK_FOR_GL_ERROR();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GLContext::GLInternalColorFormat(canvas->GetColorFormat()), 
                     0, 0, canvas->GetWidth(), canvas->GetHeight(), 0);
    CHECK_FOR_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_BLEND);
}

void GLRenderer::Render(Canvas3D* canvas) {
    // logger.info << "render c3d: " << canvas << logger.end;
    // @todo: assert we are in preprocess stage
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the screen and the depth buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    IViewingVolume* volume = canvas->GetViewingVolume();
    // If no viewing volume is set for the viewport ignore it.
    if (volume != NULL) {
        volume->SignalRendering(arg.approx);

        // Set viewport size 
        Vector<4,int> d(0, 0, canvas->GetWidth(), canvas->GetHeight());
        glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
        CHECK_FOR_GL_ERROR();

        // apply the volume
        ApplyViewingVolume(*volume);
    }
    CHECK_FOR_GL_ERROR();

    // run the processing phases
    RenderingEventArg rarg(canvas, *this, arg.start, arg.approx);
    this->preProcess.Notify(rarg);
    
    rv->light = lv->GetLights().at(0);
    
    // this->stage = RENDERER_PROCESS;
    this->process.Notify(rarg);
    // this->stage = RENDERER_POSTPROCESS;
    this->postProcess.Notify(rarg);
    // this->stage = RENDERER_PREPROCESS;

    glClientActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx->LookupCanvas(canvas));
    CHECK_FOR_GL_ERROR();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GLContext::GLInternalColorFormat(canvas->GetColorFormat()), 
                     0, 0, canvas->GetWidth(), canvas->GetHeight(), 0);
    CHECK_FOR_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);
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
    GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    // this->stage = RENDERER_INITIALIZE;
    this->initialize.Notify(RenderingEventArg(NULL, *this));
    //this->stage = RENDERER_PREPROCESS;
    CHECK_FOR_GL_ERROR();
    init = true;
}
    
void GLRenderer::Handle(Core::DeinitializeEventArg arg) {

}
    
void GLRenderer::Handle(Core::ProcessEventArg arg) {
    this->arg = arg;
    canvas->Accept(*cv);
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

void GLRenderer::SetCanvas(ICanvas* canvas) {
    this->canvas = canvas;
}

ICanvas* GLRenderer::GetCanvas() {
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
