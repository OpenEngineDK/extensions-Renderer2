// Renderer interface.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE2_INTERFACE_RENDERER_H_
#define _OE2_INTERFACE_RENDERER_H_

namespace OpenEngine {
namespace Renderers2 {

using OpenEngine::Core::IEvent;
using Core::IListener;

class IRenderContext;
class Canvas3D;

/**
 * Renderer interface.
 * 
 * 
 * @class IRenderer IRenderer.h Renderers2/IRenderer.h
 */
class IRenderer {
public:
    virtual ~IRenderer() {}

    virtual void Render(Canvas3D* canvas) = 0;
};

} // NS Renderers
} // NS OpenEngine

#endif // _OE_INTERFACE_RENDERER_H_
