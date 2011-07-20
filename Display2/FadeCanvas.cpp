// Cross fade between canvases
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------


#include <Display2/FadeCanvas.h>

namespace OpenEngine {
namespace Display2 {

FadeCanvas::FadeCanvas(unsigned int width, unsigned int height)
    : CompositeCanvas(width, height)
    , fade(false)
{
}

FadeCanvas::~FadeCanvas() {
}
    
void FadeCanvas::FadeIn(ICanvas* canvas, float duration) {
    progress = 0.0;
    this->duration = duration;
    fade = true;
    canvases.clear();
    CompositeCanvas::Container& c = AddCanvas(canvas, 0, 0);
    c.opacity = 0.0;
}

void FadeCanvas::FadeTo(ICanvas* canvas, float duration) {
    progress = 0.0;
    this->duration = duration;
    fade = true;
    CompositeCanvas::Container& c = AddCanvas(canvas, 0, 0);
    c.opacity = 0.0;
}

void FadeCanvas::Handle(Core::ProcessEventArg arg) {
    if (!fade || canvases.empty()) return;
    progress += arg.approx * 1e-6;
    float scale = fmin(progress / duration, 1.0);
    if (canvases.size() > 1) {
        canvases[0].opacity = 1.0f - scale; 
        canvases[1].opacity = scale;
    }
    else {
        canvases[0].opacity = scale; 
    }
    if (progress > duration) {
        fade = false;
        canvases.erase(canvases.begin(), canvases.begin());
   }
}

}
}
