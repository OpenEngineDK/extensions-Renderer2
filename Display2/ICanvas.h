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

namespace OpenEngine {
namespace Display2 {

/**
 * Canvas interface.
 *
 * A canvas represents a two-dimensional drawable pixel surface.
 *
 * @class ICanvas ICanvas.h Display2/ICanvas.h
 */
class ICanvas {
public:
    virtual ~ICanvas() {}

    /**
     * Get canvas width.
     *
     * @return Canvas width
     */
    virtual unsigned int GetWidth() const = 0;

    /**
     * Get canvas height.
     *
     * @return Canvas height
     */
    virtual unsigned int GetHeight() const = 0;
};

} // NS Display
} // NS OpenEngine

#endif // _INTERFACE_CANVAS_H_
