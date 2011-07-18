// Composite Canvas
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_COMPOSITE_CANVAS_H_
#define _OE_COMPOSITE_CANVAS_H_

#include <Display2/ICanvas.h>
#include <Resources/ITexture2D.h>

#include <vector>
#include <set>

namespace OpenEngine {
namespace Display2 {
   
using Resources::ITexture2DPtr;
using std::vector;
using std::set;

/**
 * Composite Canvas
 *
 * For compositing static or dynamically rendered images.
 *
 * This can be done in a layered fashion (HUD) or side by side (Split
 * screen).
 *
 * @class CompositeCanvas CompositeCanvas.h Display2/CompositeCanvas.h
 */
class CompositeCanvas: public ICanvas {
protected:
    unsigned int width, height;
public:
    class Container {
    public:
        Container(ICanvas* canvas, int x, int y): canvas(canvas), x(x), y(y) {}
        ICanvas* canvas;
        int x;
        int y;
    };
    typedef vector<Container>::iterator ContainerIterator;
private:
    vector<Container> canvases;
    set<ICanvas*> visited; //optimization: only visit duplicates once.
public:
    CompositeCanvas(unsigned int width, unsigned int height);
    virtual ~CompositeCanvas();

    virtual void Accept(ICanvasVisitor& visitor);

    Container& AddCanvas(ICanvas* canvas, int x, int y);
 
    ContainerIterator CanvasesBegin();
    ContainerIterator CanvasesEnd();

    virtual void AcceptChildren(ICanvasVisitor& visitor);    

    virtual unsigned int GetWidth() { return width; } 
    virtual unsigned int GetHeight() { return height; } 

};

} // NS Display
} // NS OpenEngine

#endif // _OE_COMPOSITE_CANVAS_H_
