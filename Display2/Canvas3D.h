// Canvas 3D
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_CANVAS_3D_H_
#define _OE_CANVAS_3D_H_

#include <Display2/ICanvas.h>
#include <Math/RGBAColor.h>

namespace OpenEngine {
    namespace Scene {
        class ISceneNode;
    }
    namespace Display {
        class IViewingVolume;
    }
namespace Display2 {
   
using Display::IViewingVolume;
using Scene::ISceneNode;
using Math::RGBAColor;

/**
 * Canvas 3D
 *
 * @class Canvas3D Canvas3D.h Display2/Canvas3D.h
 */
class Canvas3D: public ICanvas {
protected:
    unsigned int width, height;
    IViewingVolume* cam;
    ISceneNode* scene;
    RGBAColor bgc;
public:
    Canvas3D(unsigned int width, unsigned int height, ColorFormat format = Resources::RGB): 
        ICanvas(format),
        width(width), height(height), cam(NULL), scene(NULL) {}

    Canvas3D(unsigned int width, unsigned int height, IViewingVolume* cam, ISceneNode* scene, ColorFormat format = Resources::RGB): 
        ICanvas(format),
        width(width), height(height), cam(cam), scene(scene) {} 
    virtual ~Canvas3D() {}

    /**
     * Set the viewing volume (camera)
     * This viewing volume will be passed to a renderer and used when
     * rendering the scene.
     *
     * @param vv The viewing volume
     */
    virtual void SetViewingVolume(IViewingVolume* cam) { this->cam = cam; }

    /**
     * Get the viewing volume (camera)
     * This viewing volume will be passed to a renderer and used when
     * rendering the scene.
     *
     * @return The viewing volume
     */
    virtual IViewingVolume* GetViewingVolume() const { return cam; }

    /**
     * Set the root scene graph node.
     * The scene root will be passed to a renderer and used when
     * rendering the scene.
     *
     * @param scene The scene graph root
     */
    virtual void SetScene(ISceneNode* scene) { this->scene = scene; }

    /**
     * Get the root scene graph node.
     * The scene root will be passed to a renderer and used when
     * rendering the scene.
     *
     * @return The scene graph root
     */
    virtual ISceneNode* GetScene() const { return scene; }

    virtual void Accept(ICanvasVisitor& visitor) { visitor.Visit(this); }

    virtual unsigned int GetWidth() { return width; } 
    virtual unsigned int GetHeight() { return height; } 

    void SetBackgroundColor(RGBAColor color) {
        bgc = color;
    }
    
    RGBAColor GetBackgroundColor() {
        return bgc;
    }



};

} // NS Display
} // NS OpenEngine

#endif // _OE_CANVAS_H_
