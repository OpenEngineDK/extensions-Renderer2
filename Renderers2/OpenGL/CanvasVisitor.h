// OpenGL Canvas visitor
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_OPENGL_CANVAS_VISITOR_H_
#define _OE_OPENGL_CANVAS_VISITOR_H_

#include <Display2/ICanvas.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

class GLRenderer;

using Display2::ICanvasVisitor;
using Display2::Canvas3D;
using Display2::Canvas2D;
using Display2::CompositeCanvas;

/**
 * OpenGL Canvas disambiguator for compositing one or more canvases
 * into a single image.
 */
class CanvasVisitor: public ICanvasVisitor {
private:
    GLRenderer& renderer;
public:
    CanvasVisitor(GLRenderer& renderer);
    virtual ~CanvasVisitor();
    void Visit(Canvas3D* canvas);
    void Visit(CompositeCanvas* canvas);
    void Visit(Canvas2D* canvas);
};

} // NS OpenGL
} // NS Renderers2
} // NS OpenEngine

#endif // _OE_OPENGL_CANVAS_VISITOR_H_
