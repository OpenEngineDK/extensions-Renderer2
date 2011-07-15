// OpenEngine Shader Resource
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources2/ShaderResource.h>

#include <Resources/File.h>
#include <Resources/ResourceManager.h>
#include <Logging/Logger.h>
#include <Core/Exceptions.h>
#include <string>

namespace OpenEngine {    
namespace Resources2 {

using namespace Resources;
using namespace Core;
using namespace std;

ShaderResourcePlugin::ShaderResourcePlugin(): locked(false) {
    this->AddExtension("glsl");
}

ShaderResourcePlugin::~ShaderResourcePlugin() {

}

ShaderResourcePtr ShaderResourcePlugin::CreateResource(string file) {
    ShaderResource* shader = new ShaderResource(*this, file);
    return ShaderResourcePtr(shader);
}

ifstream& ShaderResourcePlugin::RequestResource(string filename, ShaderResource& shader) {
    // logger.info << "request file: " << filename << logger.end;
    ifstream* f = File::Open(filename);
    if (!locked) {
        events[filename].Detach(shader);
        events[filename].Attach(shader);
    }
    streams[filename] = f;
    
    map<string, DateTime>::iterator it = timestamps.find(filename);
    if (it == timestamps.end())
        timestamps[filename] = File::GetLastModified(filename);
    return *f;
}

void ShaderResourcePlugin::ReleaseResource(string filename) {
    map<string, ifstream*>::iterator it = streams.find(filename);
    if (it != streams.end()) {
        ifstream* f = it->second;
        f->close();
        delete f;
        streams.erase(it);
    }
}
    
void ShaderResourcePlugin::Detach(ShaderResource& shader) {
    
    map<string, Event<ShaderResourcePlugin::ChangedEventArg> >::iterator it = events.begin();
    for (; it != events.end(); ++it) {
        it->second.Detach(shader);
        if (it->second.Size() == 0)
            events.erase(it);
    }
}

void ShaderResourcePlugin::Handle(Core::ProcessEventArg arg) {
    map<string, DateTime>::iterator it = timestamps.begin();
    for (; it != timestamps.end(); ++it) {
        string filename = it->first;
        
        DateTime timestamp = File::GetLastModified(filename);

        if (timestamp != timestamps[filename]) {
            timestamps[filename] = timestamp;
            logger.info << "Timestamp changed for file: " << filename << logger.end;
            map<string, Event<ShaderResourcePlugin::ChangedEventArg> >::iterator it2 = events.find(filename);
            if (it2 != events.end()) {
                // @ todo: consider moving this locking mechanism into
                // the event. Event should queue attach and detach
                // operations while notifying. Otherwise strange stuff
                // happens!
                locked = true;
                it2->second.Notify(ShaderResourcePlugin::ChangedEventArg());
                locked = false;
            }
        }
    }
}

ShaderResource::ShaderResource(ShaderResourcePlugin& plugin, string filename)
    : plugin(plugin)
    , filename(filename)
    , dir(File::Parent(filename)) {

}

ShaderResource::~ShaderResource() {
    plugin.Detach(*this);
}

string ShaderResource::LoadShader(vector<string> files) {
    string res;
    for (unsigned int i = 0; i < files.size(); ++i) {         
        // @todo: try to reduce code duplication.
        try { // assume relative path
            int sz = File::GetSize(dir + files[i]);
            char* buf = new char[sz];
            ifstream& f = plugin.RequestResource(dir + files[i], *this);
            f.read(buf, sz-1);
            buf[sz-1] = '\0';
            res += string(buf, sz);
            delete[] buf;
            plugin.ReleaseResource(dir + files[i]);
        }
        catch (ResourceException& e) { // assume absolute path
            int sz = File::GetSize(files[i]);
            char* buf = new char[sz];
            ifstream& f = plugin.RequestResource(files[i], *this);            
            f.read(buf, sz-1);
            buf[sz-1] = '\0';
            res += string(buf, sz);
            delete[] buf;
            plugin.ReleaseResource(files[i]);
        }
    }
    return res;
}

void ShaderResource::Load() {
    int sz = File::GetSize(filename);
    ifstream& f = plugin.RequestResource(filename, *this);

    unsigned int line = 0;

    vector<string> vertexShaders;
    vector<string> fragmentShaders;
    vector<string> geometryShaders;

    char* buf = new char[sz];
    char* file = new char[sz];

    // @todo: parser is very messy and filled with stack overflow
    // dangers. Fix it by allocating large enough buffers on the heap.

    // parse the glsl meta file
    while (!f.eof()){
        ++line;
        f.getline(buf, sz);
        string type = string(buf,5);

        // Empty lines and comments can be ignored.
        if (type.empty() ||
            buf[0] == '#')
            continue;
        
        if (type == "vert:") {
            if (sscanf(buf, "vert: %s", file) == 1)
                vertexShaders.push_back(file);
            else
                logger.warning << "Line("<<line<<") Invalid vertex shader." << logger.end;
        }else if (type == "geom:") {
            if (sscanf(buf, "geom: %s", file) == 1)
                geometryShaders.push_back(file);
            else
                logger.warning << "Line("<<line<<") Invalid geometry shader." << logger.end;
        }else if (type == "frag:") {
            if (sscanf(buf, "frag: %s", file) == 1)
                fragmentShaders.push_back(file);
            else
                logger.warning << "Line("<<line<<") Invalid fragment shader." << logger.end;
        } else if (type == "text:" || type == "tex2D" || type == "tex3D") {
            const int maxlength = 300;
            char fileandname[maxlength];
            if (sscanf(buf, "text: %s", fileandname) == 1 ||
                sscanf(buf, "tex2D: %s", fileandname) == 1 ||
                sscanf(buf, "tex3D: %s", fileandname) == 1) {
                
                int seperator=0;
                for(int i=0;i<maxlength;i++) {
                    if(fileandname[i]=='|')
                        seperator=i;
                    if(fileandname[i]=='\0')
                        break;
                }
                if(seperator==0)
                    throw Exception("no separator(|) between texture name and file");

                string texname = string(fileandname, seperator);
                string texfile = string(fileandname + seperator + 1);

                if (type == "text:" || type == "tex2D") {
                    ITexture2DPtr t = ResourceManager<ITexture2D>::Create(texfile);
                    SetTexture2D(texname, t);
                } else if (type == "tex3D:"){
                    throw Exception("tex3D not supported, yet ...");
                }
            }
            else{
                logger.error << "Line("<<line<<") Invalid texture resource: '" << file << "'" << logger.end;
                throw Exception("Invalid texture resource");
            }
        }else if (type == "attr:" || type == "unif:") {
            char name[255];
            float attr[4];
            int n = 0;
            if (type == "attr:")
                n = sscanf(buf, "attr: %s = %f %f %f %f", name, &attr[0], &attr[1], &attr[2], &attr[3]) - 1;
            else if (type == "unif:")
                n = sscanf(buf, "unif: %s = %f %f %f %f", name, &attr[0], &attr[1], &attr[2], &attr[3]) - 1;
            
            switch(n){
            case 1:
                GetUniform(string(name)).Set(attr[0]);
                break;
            case 2:
                GetUniform(string(name)).Set(Vector<2, float>(attr[0], attr[1]));
                break;
            case 3:
                GetUniform(string(name)).Set(Vector<3, float>(attr[0], attr[1], attr[2]));
                break;
            case 4:
                GetUniform(string(name)).Set(Vector<4, float>(attr[0], attr[1], attr[2], attr[3]));
                break;
            }
        }
    }
    
    delete[] buf;
    delete[] file;
    plugin.ReleaseResource(filename);
    // load the shaders
    vertexShader = LoadShader(vertexShaders);
    fragmentShader = LoadShader(fragmentShaders);
}

void ShaderResource::Unload() {
    vertexShader = string();
    fragmentShader = string();
}

void ShaderResource::Handle(ShaderResourcePlugin::ChangedEventArg arg) {
    Unload();
    Load();
    // fire the shader changed event arg (for notifying the shader binders (e.g. the GLContext)
    Shader::changedEvent.Notify(Shader::ChangedEventArg(this));  
}

} // NS Resources
} // NS OpenEngine
