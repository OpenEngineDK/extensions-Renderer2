// Stereo camera implementation
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _OE_STEREO_CAMERA_H_
#define _OE_STEREO_CAMERA_H_

#include <Display/PerspectiveViewingVolume.h>

namespace OpenEngine {
namespace Display2 {

using Display::PerspectiveViewingVolume;
using Display::IViewingVolume;

/**
 * 
 *
 * @class StereoCamera StereoCamera.h Display/StereoCamera.h
 */
class StereoCamera : public PerspectiveViewingVolume {
private:
    float eyedist, halfdist;
    PerspectiveViewingVolume* left;
    PerspectiveViewingVolume* right;

    void UpdateChildren() {
        Vector<3,float> pos = GetPosition();
        Quaternion<float> rot = GetDirection();
        left->SetPosition(pos + rot.RotateVector(Vector<3,float>(-halfdist,0,0)));
        left->SetDirection(rot);
        right->SetPosition(pos + rot.RotateVector(Vector<3,float>(halfdist,0,0)));
        right->SetDirection(rot);        
    }
    
public:

    StereoCamera(): eyedist(5.0), halfdist(0.5 * eyedist), left(new PerspectiveViewingVolume()), right(new PerspectiveViewingVolume()) {}

    ~StereoCamera() {
        delete left;
        delete right;
    }

    virtual void SignalRendering(const float dt) {
        left->SignalRendering(dt);
        right->SignalRendering(dt);
        PerspectiveViewingVolume::SignalRendering(dt);
    }

    IViewingVolume* GetLeft() {
        return left;
    }

    IViewingVolume* GetRight() {
        return right;
    }
    
    void SetEyeDistance(float dist) {
        eyedist = dist;
        halfdist = 0.5 * eyedist;
        UpdateChildren();
    }

    float GetEyeDistance() {
        return eyedist;
    }

    virtual void SetPosition(const Vector<3,float> position) {
        ViewingVolume::SetPosition(position);
        UpdateChildren();        
    }

    virtual void SetDirection(const Quaternion<float> direction) {
        ViewingVolume::SetDirection(direction);
        UpdateChildren();        
    }

    virtual void Update(const unsigned int width, const unsigned int height) {
        left->Update(width, height);
        right->Update(width, height);
        PerspectiveViewingVolume::Update(width, height);
    }

    virtual void SetFOV(const float fov) {
        left->SetFOV(fov);
        right->SetFOV(fov);
        PerspectiveViewingVolume::SetFOV(fov);
    }

    virtual void SetAspect(const float aspect) {
        left->SetAspect(aspect);
        right->SetAspect(aspect);
        PerspectiveViewingVolume::SetAspect(aspect);
    }

    virtual void SetNear(const float distNear) {
        left->SetNear(distNear);
        right->SetNear(distNear);
        PerspectiveViewingVolume::SetNear(distNear);
    }

    virtual void SetFar(const float distFar) {
        left->SetFar(distFar);
        right->SetFar(distFar);
        PerspectiveViewingVolume::SetFar(distFar);
    }


};

}
}

#endif // _STEREO_CAMERA_H_
