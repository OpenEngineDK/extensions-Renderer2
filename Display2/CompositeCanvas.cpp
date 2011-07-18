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
    canvases.push_back(CompositeCanvas::Container(canvas, x, y));
    return canvases.at(canvases.size() - 1);
}

void CompositeCanvas::AcceptChildren(ICanvasVisitor& visitor) {
    visited.clear();
    vector<CompositeCanvas::Container>::reverse_iterator it = canvases.rbegin();
    for (; it != canvases.rend(); ++it) {
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


} // NS Display
} // NS OpenEngine
