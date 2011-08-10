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
#include <Utils/Box.h>
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
using Utils::Box;
using std::map;
using std::string;

class Shader;
typedef boost::shared_ptr<Shader> ShaderPtr;

class Uniform {
friend class Shader;
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

    class ChangedEventArg {
    public:
        ChangedEventArg(Shader* shader, Uniform* uniform): shader(shader), uniform(uniform) {}
        virtual ~ChangedEventArg() {}
        Shader* shader;
        Uniform* uniform;
    };

private:
    Shader* shader;
    Kind kind;
    Data data;
    Uniform(Shader* shader);
public:
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
friend class Uniform;
public:    
    typedef map<string, Uniform*>::iterator UniformIterator;
    class ChangedEventArg {
    public:
        ChangedEventArg(Shader* shader): shader(shader) {}
        virtual ~ChangedEventArg() {}
        Shader* shader;
    };
private:
    map<string, Uniform*> uniforms;
    map<string, Box<IDataBlockPtr>*> attributes;
    map<string, Box<ITexture2DPtr>*> textures;
    map<string, Box<ICubemapPtr>*> cubemaps;
protected:
    string vertexShader, fragmentShader;
    Event<ChangedEventArg> changedEvent;
    Event<Uniform::ChangedEventArg> uniformChangedEvent;
public:
    Shader();
    Shader(string vertexShader, string fragmentShader);
    virtual ~Shader();

    Uniform& GetUniform(string name);
    
    Box<IDataBlockPtr>& GetAttribute(string name);
    Box<ITexture2DPtr>& GetTexture2D(string name);
    Box<ICubemapPtr>& GetCubemap(string name);

    virtual string GetVertexShader(); 
    virtual string GetFragmentShader();

    IEvent<ChangedEventArg>& ChangedEvent() { return changedEvent; }
    IEvent<Uniform::ChangedEventArg>& UniformChangedEvent() { return uniformChangedEvent; }
};

} // NS Resources
} // NS OpenEngine

#endif // _OE_SHADER_H_
