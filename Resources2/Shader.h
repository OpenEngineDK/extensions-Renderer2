// OpenEngine Shader Representation
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_SHADER_H_
#define _OE_SHADER_H_

#include <Math/Vector.h>
#include <Math/Matrix.h>
#include <Resources/IDataBlock.h>
#include <Resources/ITexture2D.h>
#include <string>
#include <map>

namespace OpenEngine {    
namespace Resources2 {

using Math::Vector;
using Math::Matrix;
using Resources::IDataBlockPtr;
using Resources::ITexture2DPtr;
using std::map;
using std::string;

class Uniform {
public:
    enum Kind {
        UNKNOWN,
        INT,
        FLOAT,
        FLOAT3,
        FLOAT4,
        MAT3X3,
        MAT4X4
    };
    struct Data {
        union {
            int i;
            float f;
            float fv[16];
        };
    };

private:
    Kind kind;
    Data data;
public:
    Uniform();
    virtual ~Uniform();

    void Set(int v);
    void Set(float v);
    void Set(Vector<3,float> v);
    void Set(Vector<4,float> v);
    void Set(Matrix<3,3,float> v);
    void Set(Matrix<4,4,float> v);

    Kind GetKind();
    const Data GetData();
};

/**
 * OpenEngine Shader Ressource
 *
 * Encapsulates the data necessary to represent a shader effect.
 *  
 * @class Shader Shader.h Resources2/Shader.h
 */
class Shader {
public:
    typedef map<string, Uniform>::iterator UniformIterator;
    typedef map<string, IDataBlockPtr>::iterator AttributeIterator;
    typedef map<string, ITexture2DPtr>::iterator Texture2DIterator;
private:
    map<string, Uniform> uniforms;
    map<string, IDataBlockPtr> attributes;
    map<string, ITexture2DPtr> textures;
    string vertexShader, fragmentShader;
public:
    Shader(string vertexShader, string fragmentShader);
    virtual ~Shader();

    Uniform& GetUniform(string name);
    
    IDataBlockPtr GetAttribute(string name);
    void SetAttribute(string name, IDataBlockPtr attr);

    ITexture2DPtr GetTexture2D(string name);
    void SetTexture2D(string name, ITexture2DPtr tex);

    virtual string GetVertexShader(); 
    virtual string GetFragmentShader();

    UniformIterator UniformsBegin();
    UniformIterator UniformsEnd();

    AttributeIterator AttributesBegin();
    AttributeIterator AttributesEnd();

    Texture2DIterator Textures2DBegin();
    Texture2DIterator Textures2DEnd();
    
};

} // NS Resources
} // NS OpenEngine

#endif // _OE_SHADER_H_
