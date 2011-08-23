// OpenGL renderer implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_OPENGL_RENDERER_H_
#define _OE_OPENGL_RENDERER_H_

#include <Core/Event.h>
#include <Core/IModule.h>
#include <Core/IListener.h>
#include <Utils/Timer.h>
#include <Math/RGBAColor.h>

#include <Renderers2/OpenGL/GLContext.h>

namespace OpenEngine {
    namespace Display {
        class IViewingVolume;
    }
    namespace Display2 {
        class ICanvas;
        class Canvas3D;
        class CompositeCanvas;
    }

    namespace Resources2 {
        class ShaderResource;
        class Shader;
        typedef boost::shared_ptr<ShaderResource> ShaderResourcePtr;
    }
namespace Renderers2 {
namespace OpenGL {

using Display2::ICanvas;
using Display2::CompositeCanvas;
using Display2::Canvas3D;
using Resources2::ShaderResourcePtr;
using Resources2::Shader;
using Utils::Time;
using Core::IModule;
using Core::IEvent;
using Core::Event;
using Math::RGBAColor;

class GLRenderer;
class RenderingView;
class LightVisitor;
class CanvasVisitor;

class RenderingEventArg {
public:
    Canvas3D* canvas;
    GLRenderer& renderer;
    Time time;
    unsigned int approx;
    RenderingEventArg(Canvas3D* canvas, GLRenderer& renderer, Time time = Time(), unsigned int approx = 0)
        : canvas(canvas), renderer(renderer), time(time), approx(approx) {}
};

/**
 * OpenGL Renderer
 *
 * @class Renderer Renderer.h Renderers2/OpenGL/Renderer.h
 */
class GLRenderer :
        public IModule {
private:
    GLContext* ctx;
    ICanvas* canvas;
    bool init;
    int level; // canvas recursion level

    RenderingView* rv;
    LightVisitor* lv;
    CanvasVisitor* cv;

    // Event lists for the rendering phases.
    Event<RenderingEventArg> initialize;
    Event<RenderingEventArg> preProcess;
    Event<RenderingEventArg> process;
    Event<RenderingEventArg> postProcess;
    Event<RenderingEventArg> deinitialize;

    void ApplyViewingVolume(Display::IViewingVolume& volume);

    void BindUniforms(GLContext::GLShader& glshader);
    void BindAttributes(GLContext::GLShader& glshader);
    void UnbindAttributes(GLContext::GLShader& glshader);
    void BindTextures2D(GLContext::GLShader& glshader);
    void UnbindTextures2D(GLContext::GLShader& glshader);

    inline void RenderSkybox(Canvas3D* canvas);

    Core::ProcessEventArg arg;
    
    ShaderResourcePtr quadShader;
    GLuint vsLoc, tcLoc, clLoc, txLoc, dimLoc;
public:
    GLRenderer(GLContext* ctx);
    virtual ~GLRenderer();

    // propably the only interface method for a renderer
    void Render(Canvas3D* canvas);
    void Render(CompositeCanvas* canvas);
    // void Render(Shader* shader, Canvas3D* canvas = NULL);

    GLuint Apply(Shader* shader);
    void Release(Shader* shader);

    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);

    /**
     * Rendering stages/phases.
     * A renderer will after construction be in the initialization
     * stage until the InitializeEvent has occurred and all listeners
     * have been processed. After that the phases will change after
     * completion of each event. Thus, if \a GetCurrentStage is invoked
     * by a listener of the \a ProcessEvent the result must be
     * \a RENDERER_PROCESS.
     */
    enum RendererStage {
		RENDERER_UNINITIALIZE,
        RENDERER_INITIALIZE,
        RENDERER_PREPROCESS,
        RENDERER_PROCESS,
        RENDERER_POSTPROCESS,
        RENDERER_DEINITIALIZE
    };

    /**
     * Event lists for the rendering phases.
     */
    IEvent<RenderingEventArg>& InitializeEvent();
    IEvent<RenderingEventArg>& PreProcessEvent();
    IEvent<RenderingEventArg>& ProcessEvent();
    IEvent<RenderingEventArg>& PostProcessEvent();
    IEvent<RenderingEventArg>& DeinitializeEvent();

    /**
     * Get the current renderer stage.
     */
    RendererStage GetCurrentStage() const {
        return stage;
    }


    void SetCanvas(ICanvas* canvas);
    ICanvas* GetCanvas();

    GLContext* GetContext();


protected:
    RendererStage stage;

};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _RENDERER_H_
