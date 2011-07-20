// Color Stereo Canvas
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_COLOR_STEREO_CANVAS_H_
#define _OE_COLOR_STEREO_CANVAS_H_

#include <Display2/StereoCanvas.h>

namespace OpenEngine {
namespace Display2 {
   
/**
 * 
 *
 * @class ColorStereoCanvas ColorStereoCanvas.h Display2/ColorStereoCanvas.h
 */
class ColorStereoCanvas: public StereoCanvas {
private:
    void Init() {
        CompositeCanvas::Container& c1 = AddCanvas(left, 0.0, 0.0);
        CompositeCanvas::Container& c2 = AddCanvas(right, 0.0, 0.0);
        
        c1.color = RGBColor(0.0, 1.0, 1.0);
        c2.color = RGBColor(1.0, 0.0, 0.0);
        c1.opacity = 1.0;
        c2.opacity = 0.5;
    }
public:
    ColorStereoCanvas(unsigned int width, unsigned int height) 
        : StereoCanvas(width/2, height/2, Resources::LUMINANCE) {
        Init();
    }

    ColorStereoCanvas(unsigned int width, unsigned int height, StereoCamera* cam, ISceneNode* scene)
        : StereoCanvas(width, height, cam, scene, Resources::LUMINANCE) {
        Init();
    } 

    virtual ~ColorStereoCanvas() {}
};

} // NS Display
} // NS OpenEngine
#endif // _OE_SPLIT_STEREO_CANVAS_H_
