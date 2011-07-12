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

Uniform::Uniform(): kind(Uniform::UNKNOWN) {
    
}

Uniform::~Uniform() {

}

void Uniform::Set(int v) {
    data.i = v;
    kind = Uniform::INT;
}
    
void Uniform::Set(float v) {
    data.f = v;
    kind = Uniform::FLOAT;
}
    
void Uniform::Set(Vector<3,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::FLOAT3;
}
    
void Uniform::Set(Vector<4,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::FLOAT4;

}

void Uniform::Set(Matrix<3,3,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::MAT3X3;
}
    
void Uniform::Set(Matrix<4,4,float> v) {
    v.ToArray(data.fv);
    kind = Uniform::MAT4X4;
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

}

Uniform& Shader::GetUniform(string name) {
    return uniforms[name];
}
    
IDataBlockPtr Shader::GetAttribute(string name) {
    map<string,IDataBlockPtr>::iterator it = attributes.find(name);
    if (it != attributes.end())
        return (*it).second;
    return IDataBlockPtr(); // hmm or raise an exception?
}

void Shader::SetAttribute(string name, IDataBlockPtr attr) {
    attributes[name] = attr;
}

void Shader::UnsetAttribute(string name) {
    map<string,IDataBlockPtr>::iterator it = attributes.find(name);
    if (it != attributes.end())
        attributes.erase(it);
}

ITexture2DPtr Shader::GetTexture2D(string name) {
    map<string, ITexture2DPtr>::iterator it = textures.find(name);
    if (it != textures.end())
        return (*it).second;
    return ITexture2DPtr(); // hmm or raise an exception?    
}

void Shader::SetTexture2D(string name, ITexture2DPtr tex) {
    textures[name] = tex;    
}

void Shader::UnsetTexture2D(string name) {
    map<string,ITexture2DPtr>::iterator it = textures.find(name);
    if (it != textures.end())
        textures.erase(it);
}

string Shader::GetVertexShader() {
    return vertexShader;
}

string Shader::GetFragmentShader() {
    return fragmentShader;
}

Shader::UniformIterator Shader::UniformsBegin() {
    return uniforms.begin();
}

Shader::UniformIterator Shader::UniformsEnd() {
    return uniforms.end();
}

Shader::AttributeIterator Shader::AttributesBegin() {
    return attributes.begin();
}

Shader::AttributeIterator Shader::AttributesEnd() {
    return attributes.end();
}

Shader::Texture2DIterator Shader::Textures2DBegin() {
    return textures.begin();
}

Shader::Texture2DIterator Shader::Textures2DEnd() {
    return textures.end();
}


} // NS Resources
} // NS OpenEngine
