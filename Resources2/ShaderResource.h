// OpenEngine Shader Resource
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_SHADER_RESOURCE_H_
#define _OE_SHADER_RESOURCE_H_

#include <Resources2/Shader.h>
#include <Resources/IResource.h>
#include <Resources/IResourcePlugin.h>
#include <Core/Event.h>
#include <Core/EngineEvents.h>
#include <Utils/DateTime.h>

#include <boost/shared_ptr.hpp>
#include <fstream>
#include <vector>

namespace OpenEngine {    
namespace Resources2 {

using Resources::IResource;
using Resources::IResourcePlugin;
using Utils::DateTime;
using Core::IListener;
using Core::Event;
using Core::ProcessEventArg;

using std::ifstream;
using std::vector;

class ShaderResource; 
typedef boost::shared_ptr<ShaderResource> ShaderResourcePtr;

// for now we move the changed event down to the plugin, so no event is
// needed on the actual resource.
class SillyEventArg {};

/**
 * OpenEngine Shader Resource Plugin
 *
 * Plugin to the resource managing system for loading shaders from
 * file.
 *
 * This is an experimental implementation for providing a general file
 * handling interface, for automatic reloading and management of
 * files. Ideally it could be generalized to a file based resource
 * manager. Long term goal is to build a layer which abstracts away
 * the origin of the resource (filesystem, network, etc.).
 *
 * @class ShaderResourcePlugin ShaderResourcePlugin.h Resources2/ShaderResourcePlugin.h
 */
class ShaderResourcePlugin : public IResourcePlugin<ShaderResource>, public IListener<Core::ProcessEventArg> {
public:
    class ChangedEventArg {
    };
private:
    map<string, Event<ChangedEventArg> > events;    //!< signal resource changed
    map<string, ifstream*> streams;
    map<string, DateTime> timestamps;
    bool locked;
public:
    ShaderResourcePlugin();
    virtual ~ShaderResourcePlugin();
    ShaderResourcePtr CreateResource(string file);
    
    void Detach(ShaderResource& shader);
    
    // maybe this can be generalized by URL argument and istream return value.
    ifstream& RequestResource(string filename, ShaderResource& shader);
    void ReleaseResource(string filename);
    
    void Handle(Core::ProcessEventArg arg);
};

/**
 * OpenEngine Shader Resource
 *
 * A shader resource is a Shader which can be loaded through the
 * Resource managing system (currently only from file).
 *
 * @see Resources2/Shader
 * @see Resources/IResource
 *
 * @class ShaderResource ShaderResource.h Resources2/ShaderResource.h
 */
class ShaderResource: public Shader, public IResource<SillyEventArg>, public IListener<ShaderResourcePlugin::ChangedEventArg> {
private:
    ShaderResourcePlugin& plugin;
    string filename, dir;
    string LoadShader(vector<string> files);
public:
    ShaderResource(ShaderResourcePlugin& plugin, string filename);
    virtual ~ShaderResource();

    void Load(); // request resource from plugin and process it
    void Unload(); // free the resource and notify plugin

    void Handle(ShaderResourcePlugin::ChangedEventArg arg);
};



} // NS Resources
} // NS OpenEngine

#endif // _OE_SHADER_RESOURCE_H_
