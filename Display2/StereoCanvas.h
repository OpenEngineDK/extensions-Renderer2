// Stereo Canvas
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_STEREO_CANVAS_H_
#define _OE_STEREO_CANVAS_H_

#include <Display2/CompositeCanvas.h>
#include <Display2/StereoCamera.h>

namespace OpenEngine {
namespace Display2 {
   
/**
 * Stereo Canvas Base class for canvases implementing stereoscopic 3D.
 * This canvas composites two Canvas3D rendered from slightly different
 * viewing angles. 
 * @class StereoCamera StereoCamera.h Display2/StereoCamera.h
 */
class StereoCanvas: public CompositeCanvas {
protected:
    Canvas3D *left, *right;
public:
    StereoCanvas(unsigned int width, unsigned int height, ColorFormat format = Resources::RGB) 
        : CompositeCanvas(width, height),
          left(new Canvas3D(width, height, format)),
          right(new Canvas3D(width, height, format)) {}

    StereoCanvas(unsigned int width, unsigned int height, StereoCamera* cam, ISceneNode* scene, ColorFormat format = Resources::RGB)
        : CompositeCanvas(width, height),
          left(new Canvas3D(width, height, cam->GetLeft(), scene, format)),
          right(new Canvas3D(width, height, cam->GetRight(), scene, format)) {}

    virtual ~StereoCanvas() {
        delete left;
        delete right;
    }

    virtual void SetStereoCamera(StereoCamera* cam) { 
        left->SetViewingVolume(cam->GetLeft()); 
        right->SetViewingVolume(cam->GetRight()); 
    }

    virtual void SetScene(ISceneNode* scene) { left->SetScene(scene); right->SetScene(scene); }

    virtual void SetBackgroundColor(RGBAColor color) {
        left->SetBackgroundColor(color); 
        right->SetBackgroundColor(color); 
        bgc = color;
    }
    
    virtual RGBAColor GetBackgroundColor() {
        return bgc;
    }

    inline void SetSkybox(const Resources::ICubemapPtr skybox) { 
        left->SetSkybox(skybox);
        right->SetSkybox(skybox);
    }

    inline Resources::ICubemapPtr GetSkybox() const { return left->GetSkybox(); }

};

} // NS Display
} // NS OpenEngine

#endif // _OE_STEREO_CANVAS_H_
