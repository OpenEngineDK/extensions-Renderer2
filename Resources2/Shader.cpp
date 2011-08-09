// OpenEngine Shader Representation
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources2/Shader.h>

namespace OpenEngine {    
namespace Resources2 {

Uniform::Uniform(Shader* shader): shader(shader), kind(Uniform::UNKNOWN) {
    
}

Uniform::~Uniform() {
}

void Uniform::Set(int v) {
    data.i = v;
    kind = Uniform::INT;
    // shader->uniformChangedEvent.Notify(Uniform::ChangedEventArg(shader, *this));
}
    
void Uniform::Set(float v) {
    data.f = v;
    kind = Uniform::FLOAT;
    // shader->uniformChangedEvent.Notify(Uniform::ChangedEventArg(shader, *this));
}
    
void Uniform::Set(Vector<2,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::FLOAT2;
    // shader->uniformChangedEvent.Notify(Uniform::ChangedEventArg(shader, *this));
}

void Uniform::Set(Vector<3,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::FLOAT3;
    // shader->uniformChangedEvent.Notify(Uniform::ChangedEventArg(shader, *this));
}
    
void Uniform::Set(Vector<4,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::FLOAT4;
    // shader->uniformChangedEvent.Notify(Uniform::ChangedEventArg(shader, *this));
}

void Uniform::Set(Matrix<3,3,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::MAT3X3;
    // shader->uniformChangedEvent.Notify(Uniform::ChangedEventArg(shader, *this));
}
    
void Uniform::Set(Matrix<4,4,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::MAT4X4;
    // shader->uniformChangedEvent.Notify(Uniform::ChangedEventArg(shader, *this));
}

Uniform::Kind Uniform::GetKind() {
    return kind;
}
    
Uniform::Data Uniform::GetData() {
    return data;
}

Shader::Shader() {
}

Shader::Shader(string vertexShader, string fragmentShader)
    : vertexShader(vertexShader)
    , fragmentShader(fragmentShader) {
}

Shader::~Shader() {
    // todo: delete boxes and uniforms and fire destroyedEvent.

}

Uniform& Shader::GetUniform(string name) {
    UniformIterator it = uniforms.find(name);
    if (it != uniforms.end()) return *it->second;
    // why all this? because we store the uniform inside the map and avoid cleanup.
    Uniform* uniform = new Uniform(this);
    uniforms.insert(make_pair(name, uniform));
    return *uniform;
}

Box<IDataBlockPtr>& Shader::GetAttribute(string name) {
    Box<IDataBlockPtr>* box = attributes[name];
    if (!box) {
        box = new Box<IDataBlockPtr>();
        attributes[name] = box;
    }
    return *box;
}

// void Shader::SetAttribute(string name, IDataBlockPtr attr) {
//     attributes[name] = attr;
// }

// void Shader::UnsetAttribute(string name) {
//     map<string,IDataBlockPtr>::iterator it = attributes.find(name);
//     if (it != attributes.end())
//         attributes.erase(it);
// }

Box<ITexture2DPtr>& Shader::GetTexture2D(string name) {
    Box<ITexture2DPtr>* box = textures[name];
    if (!box) {
        box = new Box<ITexture2DPtr>();
        textures[name] = box;
    }
    return *box;
    // map<string, Box<ITexture2DPtr>*>::iterator it = textures.find(name);
    // if (it != textures.end())
    //     return (*it).second->Get();
    // return ITexture2DPtr(); // hmm or raise an exception?    
}

// void Shader::SetTexture2D(string name, ITexture2DPtr tex) {
//     Box<ITexture2DPtr>* b = textures[name];
//     if (!b) {
//         b = new Box<ITexture2DPtr>();
//         textures[name] = b;
//     }
//     b->Set(tex);    
// }

// void Shader::UnsetTexture2D(string name) {
//     map<string, Box<ITexture2DPtr>*>::iterator it = textures.find(name);
//     if (it != textures.end()) {
//         delete it->second;
//         textures.erase(it);        
//     }
// }

Box<ICubemapPtr>& Shader::GetCubemap(string name) {
    Box<ICubemapPtr>* box = cubemaps[name];
    if (!box) {
        box = new Box<ICubemapPtr>();
        cubemaps[name] = box;
    }
    return *box;

    // map<string, ICubemapPtr>::iterator it = cubemaps.find(name);
    // if (it != cubemaps.end())
    //     return (*it).second;
    // return ICubemapPtr(); // hmm or raise an exception?    
}

// void Shader::SetCubemap(string name, ICubemapPtr cubemap) {
//     cubemaps[name] = cubemap;    
// }

// void Shader::UnsetCubemap(string name) {
//     map<string,ICubemapPtr>::iterator it = cubemaps.find(name);
//     if (it != cubemaps.end())
//         cubemaps.erase(it);
// }

string Shader::GetVertexShader() {
    return vertexShader;
}

string Shader::GetFragmentShader() {
    return fragmentShader;
}

// Shader::UniformIterator Shader::UniformsBegin() {
//     return uniforms.begin();
// }

// Shader::UniformIterator Shader::UniformsEnd() {
//     return uniforms.end();
// }

// Shader::AttributeIterator Shader::AttributesBegin() {
//     return attributes.begin();
// }

// Shader::AttributeIterator Shader::AttributesEnd() {
//     return attributes.end();
// }

// Shader::Texture2DIterator Shader::Textures2DBegin() {
//     return textures.begin();
// }

// Shader::Texture2DIterator Shader::Textures2DEnd() {
//     return textures.end();
// }

// Shader::CubemapIterator Shader::CubemapsBegin() {
//     return cubemaps.begin();
// }

// Shader::CubemapIterator Shader::CubemapsEnd() {
//     return cubemaps.end();
// }


} // NS Resources
} // NS OpenEngine
