// Split Screen Stereo Canvas
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_SPLIT_STEREO_CANVAS_H_
#define _OE_SPLIT_STEREO_CANVAS_H_

#include <Display2/StereoCanvas.h>

namespace OpenEngine {
namespace Display2 {
   
/**
 * Stereo canvas implementation displaying each eyeview in split
 * screen (for two projectors and polarization filters).
 *
 * @class SplitStereoCanvas SplitStereoCanvas.h Display2/SplitStereoCanvas.h
 */
class SplitStereoCanvas: public StereoCanvas {
public:
    SplitStereoCanvas(unsigned int width, unsigned int height) 
        : StereoCanvas(width/2, height/2) {
        AddCanvas(left, 0.0, 0.0);
        AddCanvas(right, width / 2 + 1, 0.0);
    }

SplitStereoCanvas(unsigned int width, unsigned int height, StereoCamera* cam, ISceneNode* scene)
        : StereoCanvas(width, height, cam, scene) {
        AddCanvas(left, 0.0, 0.0);
        AddCanvas(right, width / 2 + 1, 0.0);
} 

virtual ~SplitStereoCanvas() {}
};

} // NS Display
} // NS OpenEngine
#endif // _OE_SPLIT_STEREO_CANVAS_H_
