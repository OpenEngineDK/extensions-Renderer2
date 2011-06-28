// GLSL Phong shader implementation
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources2/PhongShader.h>

#include <Resources/DirectoryManager.h>
#include <Resources/File.h>
#include <Logging/Logger.h>
#include <Resources/ITexture2D.h>
#include <Geometry/GeometrySet.h>
#include <Geometry/Material.h>
#include <Geometry/Mesh.h>

namespace OpenEngine {
namespace Resources2 {

using namespace Geometry;
using Resources::File;
using Resources::DirectoryManager;

void PhongShader::AddDefine(string name) {    
    string def = string("#define ") + name + string("\n");
    vertexShader = def + vertexShader;
    fragmentShader = def + fragmentShader;
}

void PhongShader::AddDefine(string name, int val) {
    std::stringstream v;
    v << val;
    string def = string("#define ") + name + string(" ") + v.str() + string("\n");
    vertexShader = def + vertexShader;
    fragmentShader = def + fragmentShader;
}

PhongShader::PhongShader(Mesh* mesh) {

    Material* mat = mesh->GetMaterial().get();
    IDataBlockPtr tans = mesh->GetGeometrySet()->GetAttributeList("tangent");
    IDataBlockPtr bitans = mesh->GetGeometrySet()->GetAttributeList("bitangent");

    ITexture2DPtr ambient = mat->Get2DTextures()["ambient"];
    ITexture2DPtr diffuse = mat->Get2DTextures()["diffuse"];
    ITexture2DPtr specular = mat->Get2DTextures()["specular"];
    ITexture2DPtr bump = mat->Get2DTextures()["normal"];
    ITexture2DPtr opacity = mat->Get2DTextures()["opacity"];
    if (!bump)
        bump = mat->Get2DTextures()["height"];
    if (bump && bump->GetChannels() < 3) bump.reset();

    const string vertexFile = DirectoryManager::FindFileInPath("extensions/OpenGLRenderer/shaders/PhongShader.glsl.vert");
    const string fragmentFile = DirectoryManager::FindFileInPath("extensions/OpenGLRenderer/shaders/PhongShader.glsl.frag");

    int sz = File::GetSize(vertexFile);
    char* buf = new char[sz];
    ifstream* f = File::Open(vertexFile);
    f->read(buf, sz-1);
    buf[sz-1] = '\0';
    f->close();
    vertexShader = string(buf, sz);
    delete buf;
    delete f;

    sz = File::GetSize(fragmentFile);
    buf = new char[sz];
    f = File::Open(fragmentFile);
    f->read(buf, sz);
    buf[sz-1] = '\0';
    f->close();
    fragmentShader = string(buf, sz);
    delete buf;
    delete f;

    AddDefine("NUM_LIGHTS", 1);

    // concatenate shaders with defines
    if (ambient) {
        AddDefine("AMBIENT_MAP");
        SetTexture2D("ambientMap", ambient);
    }
    
    if (diffuse) {
        AddDefine("DIFFUSE_MAP");
        AddDefine("DIFFUSE_INDEX", mat->GetUVIndex(diffuse));
        SetTexture2D("diffuseMap", diffuse);

        // logger.info << "diffuse index: " << mat->GetUVIndex(diffuse) << logger.end;
    }

    if (specular) {
        AddDefine("SPECULAR_MAP");
        SetTexture2D("specularMap", specular);
    }

    if (bump && tans && bitans) {
        // logger.info << "bump channels: " << (unsigned int)bump->GetChannels() << logger.end;
        AddDefine("BUMP_MAP");
        AddDefine("BUMP_INDEX", mat->GetUVIndex(bump));
        SetTexture2D("bumpMap", bump);

        // logger.info << "bump index: " << mat->GetUVIndex(bump) << logger.end;
    }    

    if (opacity) {
        AddDefine("OPACITY_MAP");
        AddDefine("OPACITY_INDEX", mat->GetUVIndex(opacity));
        SetTexture2D("opacityMap", opacity);
        // logger.info << "opacity index: " << mat->GetUVIndex(diffuse) << logger.end;
    }

    if (bump && tans && bitans) {
        SetAttribute("tangent", tans);
        SetAttribute("bitangent", bitans);
    }

    logger.info << vertexShader << logger.end;
    logger.info << fragmentShader << logger.end;
}

PhongShader::~PhongShader() {

}

}
}

