// Canvas 2D
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_CANVAS_2D_H_
#define _OE_CANVAS_2D_H_

#include <Display2/ICanvas.h>
#include <Resources/ITexture2D.h>

namespace OpenEngine {
namespace Display2 {

using Resources::ITexture2DPtr;   

/**
 * Canvas 2D
 *
 * Simply a Texture2D wrapper class.
 *
 * @class Canvas2D Canvas2D.h Display2/Canvas2D.h
 */
class Canvas2D: public ICanvas {
private:
    ITexture2DPtr texture;
public:
    Canvas2D(ITexture2DPtr texture): 
        texture(texture) {}

    virtual ~Canvas2D() {}

    virtual void Accept(ICanvasVisitor& visitor) { visitor.Visit(this); }

    virtual unsigned int GetWidth() { return texture->GetWidth(); } 
    virtual unsigned int GetHeight() { return texture->GetHeight(); } 
    
    ITexture2DPtr GetTexture() { return texture; }

};

} // NS Display
} // NS OpenEngine

#endif // _OE_CANVAS_2D_H_
