// Cross fade between canvases
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Publicc License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_FADE_CANVAS_H_
#define _OE_FADE_CANVAS_H_

#include <Display2/CompositeCanvas.h>
#include <Core/IListener.h>
#include <Core/EngineEvents.h>

namespace OpenEngine {
namespace Display2 {

using Core::IListener;

class FadeCanvas: public CompositeCanvas, public IListener<Core::ProcessEventArg> {
private:
    bool fade;
    float progress, duration;    
public:
    FadeCanvas(unsigned int width, unsigned int height);
    virtual ~FadeCanvas();
    
    void FadeIn(ICanvas* canvas, float duration);
    void FadeTo(ICanvas* canvas, float duration);

    void Handle(Core::ProcessEventArg arg);
};

}
}
#endif //_OE_FADE_CANVAS_H_
