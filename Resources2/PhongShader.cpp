// GLSL Phong shader implementation
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
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
#include <Resources/ResourceManager.h>
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
    // vertexShader = def + vertexShader;
    // fragmentShader = def + fragmentShader;
    defines += def;
}

void PhongShader::AddDefine(string name, int val) {
    std::stringstream v;
    v << val;
    string def = string("#define ") + name + string(" ") + v.str() + string("\n");
    // vertexShader = def + vertexShader;
    // fragmentShader = def + fragmentShader;
    defines += def;
}

PhongShader::PhongShader(Mesh* mesh)
    : ShaderResource(*ResourceManager<ShaderResource>::Create("extensions/Renderer2/shaders/PhongShaderESCompatible.glsl").get())
{
    ShaderResource::Load();
    // logger.info << "phong" << logger.end;

    Material* mat = mesh->GetMaterial().get();
    IDataBlockPtr tans = mesh->GetGeometrySet()->GetAttributeList("tangent");
    IDataBlockPtr bitans = mesh->GetGeometrySet()->GetAttributeList("bitangent");

    ITexture2DPtr ambient = mat->Get2DTextures()["ambient"];
    ITexture2DPtr diffuse = mat->Get2DTextures()["diffuse"];
    ITexture2DPtr specular = mat->Get2DTextures()["specular"];
    ITexture2DPtr bump = mat->Get2DTextures()["normal"];
    ITexture2DPtr opacity = mat->Get2DTextures()["opacity"];
    if (!bump) {
        bump = mat->Get2DTextures()["height"]; 
    }
    if (bump) {
        bump->Load();
        if (bump->GetChannels() < 3) {
            // logger.info << "Dropping bump map, channels: " << (int)bump->GetChannels() << logger.end;
            bump.reset(); 
        }
    }

    // const string vertexFile = DirectoryManager::FindFileInPath("extensions/Renderer2/shaders/PhongShader.glsl.vert");
    // const string fragmentFile = DirectoryManager::FindFileInPath("extensions/Renderer2/shaders/PhongShader.glsl.frag");

    // const string vertexFile = DirectoryManager::FindFileInPath("extensions/Renderer2/shaders/PhongShaderESCompatible.glsl.vert");
    // const string fragmentFile = DirectoryManager::FindFileInPath("extensions/Renderer2/shaders/PhongShaderESCompatible.glsl.frag");

    // int sz = File::GetSize(vertexFile);
    // char* buf = new char[sz];
    // ifstream* f = File::Open(vertexFile);
    // f->read(buf, sz-1);
    // buf[sz-1] = '\0';
    // f->close();
    // vertexShader = string(buf, sz);
    // delete buf;
    // delete f;

    // sz = File::GetSize(fragmentFile);
    // buf = new char[sz];
    // f = File::Open(fragmentFile);
    // f->read(buf, sz);
    // buf[sz-1] = '\0';
    // f->close();
    // fragmentShader = string(buf, sz);
    // delete buf;
    // delete f;

    // concatenate shaders with defines
    if (ambient) {
        AddDefine("AMBIENT_MAP");
        AddDefine("AMBIENT_INDEX", mat->GetUVIndex(ambient));
        SetTexture2D("ambientMap", ambient);

    //     logger.info << "ambient index: " << mat->GetUVIndex(ambient) << logger.end;
    }
    
    if (diffuse) {
        AddDefine("DIFFUSE_MAP");
        AddDefine("DIFFUSE_INDEX", mat->GetUVIndex(diffuse));
        SetTexture2D("diffuseMap", diffuse);
        // logger.info << "diffuse index: " << mat->GetUVIndex(diffuse) << logger.end;
    }

    if (specular) {
        AddDefine("SPECULAR_MAP");
        AddDefine("SPECULAR_INDEX", mat->GetUVIndex(specular));
        SetTexture2D("specularMap", specular);

        // logger.info << "specular index: " << mat->GetUVIndex(specular) << logger.end;
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

    AddDefine("NUM_LIGHTS", 1);

    map<string, IDataBlockPtr> attribs = mesh->GetGeometrySet()->GetAttributeLists();
    map<string, IDataBlockPtr>::iterator itr1 = attribs.begin();
    
    for (; itr1 != attribs.end(); ++itr1) {
        // logger.info << "attrib: " << itr->first << logger.end;
        SetAttribute(itr1->first, itr1->second);
    }

    Resources::IDataBlockList tcs = mesh->GetGeometrySet()->GetTexCoords();
    Resources::IDataBlockList::iterator itr2 = tcs.begin();

    
    unsigned int count = 0;

    if (tcs.size() == 1) {
        SetAttribute("texCoord", *itr2);        
        count = 1;
    }
    else {
        for (; itr2 != tcs.end(); ++itr2) {
            SetAttribute("texCoord[" + Utils::Convert::ToString<unsigned int>(count) + "]", *itr2);
            ++count;
        }
    }
    if (count > 0) {
        AddDefine("USE_TEXTURES");
        AddDefine("NUM_TEXTURES", count);
        if (count == 1) {
            AddDefine("ONE_TEXTURE");
        }
    }
    
    if (!(bump && tans && bitans)) {
        UnsetAttribute("tangent");
        UnsetAttribute("bitangent");
    }

    // set material

    if (!ambient) GetUniform("frontMaterial.ambient").Set(mat->ambient);
    if (!diffuse) GetUniform("frontMaterial.diffuse").Set(mat->diffuse);
    if (!specular) GetUniform("frontMaterial.specular").Set(mat->specular);
    GetUniform("frontMaterial.shininess").Set(mat->shininess);
    
    // logger.info << vertexShader << logger.end;
    // logger.info << fragmentShader << logger.end;
}

PhongShader::~PhongShader() {

}

void PhongShader::SetModelViewMatrix(Matrix<4,4,float> m) {
    //Optimization: store the uniforms
    GetUniform("modelViewMatrix").Set(m);
    GetUniform("normalMatrix").Set(m.GetReduced().GetInverse().GetTranspose());
}

void PhongShader::SetModelViewProjectionMatrix(Matrix<4,4,float> m) {
    //Optimization: store the uniform
    GetUniform("modelViewProjectionMatrix").Set(m);
}

void PhongShader::SetLight(LightVisitor::LightSource l, Vector<4,float> globalAmbient) {
    // logger.info << "l.pos: " << l.position << logger.end;
    // logger.info << "l.amb: " << l.ambient << logger.end;
    // logger.info << "l.dif: " << l.diffuse << logger.end;
    // logger.info << "l.spec: " << l.specular << logger.end;
    // logger.info << "l.const: " << l.constantAttenuation << logger.end;
    // logger.info << "l.linear: " << l.linearAttenuation << logger.end;
    // logger.info << "l.quad: " << l.quadraticAttenuation << logger.end;
    GetUniform("globalAmbient").Set(globalAmbient);
    GetUniform("lightSource[0].position").Set(l.position);
    GetUniform("lightSource[0].ambient").Set(l.ambient);
    GetUniform("lightSource[0].diffuse").Set(l.diffuse);
    GetUniform("lightSource[0].specular").Set(l.specular);
    GetUniform("lightSource[0].constantAttenuation").Set(l.constantAttenuation);
    GetUniform("lightSource[0].linearAttenuation").Set(l.linearAttenuation);
    GetUniform("lightSource[0].quadraticAttenuation").Set(l.quadraticAttenuation);
}

string PhongShader::GetVertexShader() {
    return defines + vertexShader;
}
    
string PhongShader::GetFragmentShader() {
    return defines + fragmentShader;
}

}
}

