// OpenGL Canvas visitor
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers2/OpenGL/CanvasVisitor.h>
#include <Renderers2/OpenGL/GLRenderer.h>
#include <Renderers2/OpenGL/GLContext.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {


CanvasVisitor::CanvasVisitor(GLRenderer& renderer): renderer(renderer) {
  
}

CanvasVisitor::~CanvasVisitor() {

}

void CanvasVisitor::Visit(Canvas2D* canvas) {
    renderer.GetContext()->LookupCanvas(canvas);
}

void CanvasVisitor::Visit(Canvas3D* canvas) {
    renderer.Render(canvas);
}

void CanvasVisitor::Visit(CompositeCanvas* canvas) {
    renderer.Render(canvas);
}



} // NS OpenGL
} // NS Renderers2
} // NS OpenEngine

