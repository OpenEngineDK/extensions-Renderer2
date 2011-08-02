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
#include <Resources/ICubemap.h>
#include <Core/Event.h>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace OpenEngine {    
namespace Resources2 {

using Math::Vector;
using Math::Matrix;
using Resources::IDataBlockPtr;
using Resources::ITexture2DPtr;
using Resources::ICubemapPtr;
using Core::IEvent;
using Core::Event;
using std::map;
using std::string;

class Shader;
typedef boost::shared_ptr<Shader> ShaderPtr;

class Uniform {
public:
    enum Kind {
        UNKNOWN,
        INT,
        FLOAT,
        FLOAT2,
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
    void Set(Vector<2,float> v);
    void Set(Vector<3,float> v);
    void Set(Vector<4,float> v);
    void Set(Matrix<3,3,float> v);
    void Set(Matrix<4,4,float> v);

    Kind GetKind();
    Data GetData();
};

/**
 * OpenEngine Shader
 *
 * Encapsulates the data necessary to represent a shader effect.
 * Render contexts must handle GPU allocation and deallocation. 
 *
 * @see Renderers2/OpenGL/GLContext
 *
 * @class Shader Shader.h Resources2/Shader.h
 */
class Shader {
public:    
    typedef map<string, Uniform>::iterator UniformIterator;
    typedef map<string, IDataBlockPtr>::iterator AttributeIterator;
    typedef map<string, ITexture2DPtr>::iterator Texture2DIterator;
    typedef map<string, ICubemapPtr>::iterator CubemapIterator;
    class ChangedEventArg {
    public:
        ChangedEventArg(Shader* shader): shader(shader) {}
        virtual ~ChangedEventArg() {}
        Shader* shader;
    };
private:
    map<string, Uniform> uniforms;
    map<string, IDataBlockPtr> attributes;
    map<string, ITexture2DPtr> textures;
    map<string, ICubemapPtr> cubemaps;
protected:
    string vertexShader, fragmentShader;
    Event<ChangedEventArg> changedEvent;
public:
    Shader();
    Shader(string vertexShader, string fragmentShader);
    virtual ~Shader();

    Uniform& GetUniform(string name);
    
    IDataBlockPtr GetAttribute(string name);
    void SetAttribute(string name, IDataBlockPtr attr);
    void UnsetAttribute(string name);

    ITexture2DPtr GetTexture2D(string name);
    void SetTexture2D(string name, ITexture2DPtr tex);
    void UnsetTexture2D(string name);

    void SetCubemap(string name, ICubemapPtr tex);
    void UnsetCubemap(string name);

    virtual string GetVertexShader(); 
    virtual string GetFragmentShader();

    UniformIterator UniformsBegin();
    UniformIterator UniformsEnd();

    AttributeIterator AttributesBegin();
    AttributeIterator AttributesEnd();

    Texture2DIterator Textures2DBegin();
    Texture2DIterator Textures2DEnd();

    CubemapIterator CubemapsBegin();
    CubemapIterator CubemapsEnd();

    IEvent<ChangedEventArg>& ChangedEvent() { return changedEvent; }
};

} // NS Resources
} // NS OpenEngine

#endif // _OE_SHADER_H_
