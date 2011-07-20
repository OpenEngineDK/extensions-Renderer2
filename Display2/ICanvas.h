// Canvas interface.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_INTERFACE_CANVAS_H_
#define _OE_INTERFACE_CANVAS_H_

#include <Core/Exceptions.h>
#include <Resources/ITexture.h>

namespace OpenEngine {
namespace Display2 {

using Core::Exception;
using Resources::ColorFormat;

class ICanvas;
class Canvas3D;
class CompositeCanvas;
class Canvas2D;

class ICanvasVisitor {
public:
    virtual ~ICanvasVisitor() {};

    virtual void Visit(ICanvas* canvas) {
        throw Exception("Unknown Canvas type.");
    }

    virtual void Visit(Canvas3D* canvas) {}
    virtual void Visit(CompositeCanvas* canvas) {}
    virtual void Visit(Canvas2D* canvas) {}
};

/**
 * Canvas interface.
 *
 * A canvas represents a two-dimensional drawable pixel surface.
 *
 * @class ICanvas ICanvas.h Display2/ICanvas.h
 */
class ICanvas {
protected:
    ColorFormat format;
public:
    ICanvas(ColorFormat format = Resources::RGB): format(format) {}
    
    virtual ~ICanvas() {}

    /**
     * Get canvas width.
     *
     * @return Canvas width
     */
    virtual unsigned int GetWidth() = 0;

    /**
     * Get canvas height.
     *
     * @return Canvas height
     */
    virtual unsigned int GetHeight() = 0;

    /**
     * Visitor pattern accept method
     *
     * @param the canvas visitor instance.
     */
    virtual void Accept(ICanvasVisitor& visitor) = 0;

    virtual ColorFormat GetColorFormat() {
        return format;
    }
};

} // NS Display
} // NS OpenEngine

#endif // _INTERFACE_CANVAS_H_
