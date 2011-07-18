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

namespace OpenEngine {
    namespace Display {
        class IViewingVolume;
    }
    namespace Display2 {
        class ICanvas;
        class Canvas3D;
        class CompositeCanvas;
    }
namespace Renderers2 {
namespace OpenGL {

using Display2::ICanvas;
using Display2::CompositeCanvas;
using Display2::Canvas3D;
using Utils::Time;
using Core::IModule;
using Core::IEvent;
using Core::Event;
using Math::RGBAColor;

class GLContext;
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
    RGBAColor bgc;
    GLContext* ctx;
    ICanvas* canvas;
    bool init;

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

    Core::ProcessEventArg arg;
public:
    GLRenderer(GLContext* ctx);
    virtual ~GLRenderer();

    // propably the only interface method for a renderer
    void Render(Canvas3D* canvas);
    void Render(CompositeCanvas* canvas);

    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);

    /**
     * Event lists for the rendering phases.
     */
    IEvent<RenderingEventArg>& InitializeEvent();
    IEvent<RenderingEventArg>& PreProcessEvent();
    IEvent<RenderingEventArg>& ProcessEvent();
    IEvent<RenderingEventArg>& PostProcessEvent();
    IEvent<RenderingEventArg>& DeinitializeEvent();

    void SetBackgroundColor(RGBAColor color);
    RGBAColor GetBackgroundColor();

    void SetCanvas(ICanvas* canvas);
    ICanvas* GetCanvas();

    GLContext* GetContext();

};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _RENDERER_H_
