// Composite Canvas
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Display2/CompositeCanvas.h>

namespace OpenEngine {
namespace Display2 {
    
CompositeCanvas::CompositeCanvas(unsigned int width, unsigned int height)
    : width(width), height(height) {
}

CompositeCanvas::~CompositeCanvas() {

}

void CompositeCanvas::Accept(ICanvasVisitor& visitor) { 
    visitor.Visit(this); 
}

CompositeCanvas::Container& CompositeCanvas::AddCanvas(ICanvas* canvas, int x, int y) {
    canvases.push_back(CompositeCanvas::Container(canvas, x, y, canvas->GetWidth(), canvas->GetHeight()));
    return canvases.at(canvases.size() - 1);
}

void CompositeCanvas::AcceptChildren(ICanvasVisitor& visitor) {
    visited.clear();
    vector<CompositeCanvas::Container>::iterator it = canvases.begin();
    for (; it != canvases.end(); ++it) {
        if (visited.find(it->canvas) == visited.end()) {
            it->canvas->Accept(visitor);
            visited.insert(it->canvas);
        }
    }
}

CompositeCanvas::ContainerIterator CompositeCanvas::CanvasesBegin() {
    return canvases.begin();
}

CompositeCanvas::ContainerIterator CompositeCanvas::CanvasesEnd() {
    return canvases.end();
}

unsigned int CompositeCanvas::Size() {
    return canvases.size();
}

void CompositeCanvas::SetBackgroundColor(RGBAColor color) {
    bgc = color;
}
    
RGBAColor CompositeCanvas::GetBackgroundColor() {
    return bgc;
}


} // NS Display
} // NS OpenEngine
