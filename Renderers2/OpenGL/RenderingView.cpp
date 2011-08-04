// OpenGL rendering view.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers2/OpenGL/RenderingView.h>

#include <Display2/Canvas3D.h>
#include <Display/IViewingVolume.h>
#include <Resources2/Shader.h>
#include <Resources2/PhongShader.h>
#include <Scene/TransformationNode.h>
#include <Scene/RenderStateNode.h>
#include <Scene/MeshNode.h>
#include <Geometry/GeometrySet.h>
#include <Geometry/Mesh.h>
#include <Geometry/Material.h>
#include <Logging/Logger.h>


namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using namespace Math;
using namespace Geometry;
using namespace Scene;

using Resources2::Uniform;
using Resources2::PhongShader;

/**
 * Rendering view constructor.
 *
 * @param viewport Viewport in which to render.
 */
RenderingView::RenderingView()
    : ctx(NULL)
    , currentRenderState(new RenderStateNode())
    , renderTexture(true)
    , renderShader(true)
{
    currentRenderState = new RenderStateNode();
    currentRenderState->EnableOption(RenderStateNode::TEXTURE);
    currentRenderState->EnableOption(RenderStateNode::SHADER);
    currentRenderState->EnableOption(RenderStateNode::BACKFACE);
    currentRenderState->EnableOption(RenderStateNode::DEPTH_TEST);
    currentRenderState->DisableOption(RenderStateNode::LIGHTING); 
    currentRenderState->DisableOption(RenderStateNode::WIREFRAME);
}

RenderingView::~RenderingView() {}

void RenderingView::Handle(RenderingEventArg arg) {
#if OE_SAFE
    if (arg.canvas->GetScene() == NULL) 
        throw Exception("Scene was NULL while rendering.");
#endif

    map<Mesh*, PhongShader*>::iterator itr = shaders.begin();
    for (; itr != shaders.end(); ++itr) {
        itr->second->SetLight(light, Vector<4,float>(0.3, 0.3, 0.3, 1.0));
    }

    modelViewMatrix = arg.canvas->GetViewingVolume()->GetViewMatrix();
    projectionMatrix = arg.canvas->GetViewingVolume()->GetProjectionMatrix();
    ctx = arg.renderer.GetContext();

    // Setup skybox
    if (arg.canvas->GetSkybox())
        RenderSkybox(*(arg.canvas), *ctx);

    // setup default render state
    ApplyRenderState(currentRenderState);
    arg.canvas->GetScene()->Accept(*this);
    
    // process transparent meshes
    vector<RenderObject>::iterator it = transparencyQueue.begin();
    for (; it != transparencyQueue.end(); ++it) {
        RenderMesh(it->mesh, it->modelViewMatrix);
    }         
    transparencyQueue.clear();

    ctx = NULL;
}
            
void RenderingView::ApplyRenderState(RenderStateNode* node) {
    if (node->IsOptionEnabled(RenderStateNode::WIREFRAME)) {
#ifndef OE_IOS
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::WIREFRAME)) {
#ifndef OE_IOS
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
        CHECK_FOR_GL_ERROR();
    }

    if (node->IsOptionEnabled(RenderStateNode::BACKFACE)) {
        glDisable(GL_CULL_FACE);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::BACKFACE)) {
        glEnable(GL_CULL_FACE);
        CHECK_FOR_GL_ERROR();
    }
    if (node->IsOptionEnabled(RenderStateNode::DEPTH_TEST)) {
        glEnable(GL_DEPTH_TEST);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::DEPTH_TEST)) {
        glDisable(GL_DEPTH_TEST);
        CHECK_FOR_GL_ERROR();
    }

#if FIXED_FUNCTION
    if (node->IsOptionEnabled(RenderStateNode::LIGHTING)) {
        glEnable(GL_LIGHTING);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::LIGHTING)) {
        glDisable(GL_LIGHTING);
        CHECK_FOR_GL_ERROR();
    }
    if (node->IsOptionEnabled(RenderStateNode::COLOR_MATERIAL)) {
        glEnable(GL_COLOR_MATERIAL);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::COLOR_MATERIAL)) {
        glDisable(GL_COLOR_MATERIAL);
        CHECK_FOR_GL_ERROR();
    }

    if (node->IsOptionEnabled(RenderStateNode::TEXTURE))
        renderTexture = true;
    else if (node->IsOptionDisabled(RenderStateNode::TEXTURE))
        renderTexture = false;

    if (node->IsOptionEnabled(RenderStateNode::SHADER))
        renderShader = true;
    else if (node->IsOptionDisabled(RenderStateNode::SHADER))
        renderShader = false;
#endif

    // if (node->IsOptionEnabled(RenderStateNode::BINORMAL))
    //     renderBinormal = true;
    // else if (node->IsOptionDisabled(RenderStateNode::BINORMAL))
    //     renderBinormal = false;

    // if (node->IsOptionEnabled(RenderStateNode::TANGENT))
    //     renderTangent = true;
    // else if (node->IsOptionDisabled(RenderStateNode::TANGENT))
    //     renderTangent = false;

    // if (node->IsOptionEnabled(RenderStateNode::SOFT_NORMAL))
    //     renderSoftNormal = true;
    // else if (node->IsOptionDisabled(RenderStateNode::SOFT_NORMAL))
    //     renderSoftNormal = false;

    // if (node->IsOptionEnabled(RenderStateNode::HARD_NORMAL))
    //     renderHardNormal = true;
    // else if (node->IsOptionDisabled(RenderStateNode::HARD_NORMAL))
    //     renderHardNormal = false;

}


/**
 * Process a render state node.
 *
 * @param node Render state node to apply.
 */
void RenderingView::VisitRenderStateNode(Scene::RenderStateNode* node) {
    // save old state
    RenderStateNode* prevCurrent = currentRenderState;

    // apply combined render state
    currentRenderState = currentRenderState->GetCombined(*node);
    ApplyRenderState(currentRenderState);

    // visit sub tree
    node->VisitSubNodes(*this);

    // restore previous state
    delete currentRenderState;
    currentRenderState = prevCurrent;
    ApplyRenderState(currentRenderState);

    CHECK_FOR_GL_ERROR();
}

/**
 * Process a transformation node.
 *
 * @param node Transformation node to apply.
 */
void RenderingView::VisitTransformationNode(TransformationNode* node) {

    Matrix<4,4,float> m = node->GetTransformationMatrix();
    Matrix<4,4,float> oldMv = modelViewMatrix;
    modelViewMatrix = m * modelViewMatrix;

#if FIXED_FUNCTION
    // float f[16];
    // m.ToArray(f);
    // glPushMatrix();
    // glMultMatrixf(f);
    // CHECK_FOR_GL_ERROR();
#endif

    node->VisitSubNodes(*this);
    CHECK_FOR_GL_ERROR();

#if FIXED_FUNCTION
    // glPopMatrix();
    // CHECK_FOR_GL_ERROR();
#endif
    
    modelViewMatrix = oldMv;
}

void RenderingView::BindUniforms(GLContext::GLShader& glshader) {
    map<Uniform*, GLint>::iterator it = glshader.uniforms.begin();
    for (; it != glshader.uniforms.end(); ++it) {
        GLint loc = it->second; 
        Uniform& uniform = *it->first;
        const Uniform::Data data = uniform.GetData();
        switch (uniform.GetKind()) {
        case Uniform::INT:
            glUniform1i(loc, data.i);
            break;
        case Uniform::FLOAT:
            glUniform1f(loc, data.f);
            break;
        case Uniform::FLOAT2:
            glUniform2fv(loc, 1, data.fv);
            break;
        case Uniform::FLOAT3:
            glUniform3fv(loc, 1, data.fv);
            break;
        case Uniform::FLOAT4:
            glUniform4fv(loc, 1, data.fv);
            break;
        case Uniform::MAT3X3:
            glUniformMatrix3fv(loc, 1, false, data.fv);
            break;
        case Uniform::MAT4X4:
            glUniformMatrix4fv(loc, 1, false, data.fv);
            break;            
        case Uniform::UNKNOWN:
#if OE_SAFE
            throw Exception("Unknown uniform kind.");
#endif
            break;
        }
        CHECK_FOR_GL_ERROR();
    }
}

void RenderingView::BindAttributes(GLContext::GLShader& glshader) {
    map<IDataBlockPtr, GLint>::iterator it = glshader.attributes.begin();
    for (; it != glshader.attributes.end(); ++it) {
        GLint loc = it->second;
        IDataBlock* db = it->first.get();
        if (ctx->VBOSupport()) {
            glBindBuffer(GL_ARRAY_BUFFER, ctx->LookupVBO(db));
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, db->GetDimension(), db->GetType(), 0, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else {
            glEnableVertexAttribArray(loc);
            IDataBlock* db = it->first.get();
            glVertexAttribPointer(loc, db->GetDimension(), db->GetType(), 0, 0, db->GetVoidData());
        }
        CHECK_FOR_GL_ERROR();
    }
}

void RenderingView::UnbindAttributes(GLContext::GLShader& glshader) {
    map<IDataBlockPtr, GLint>::iterator it = glshader.attributes.begin();
    for (; it != glshader.attributes.end(); ++it) {
        glDisableVertexAttribArray(it->second);
        CHECK_FOR_GL_ERROR();
    }
}

void RenderingView::BindTextures2D(GLContext::GLShader& glshader) {
    map<ITexture2DPtr, GLint>::iterator tex_it = glshader.textures.begin();
    GLint texUnit = 0;
    for (; tex_it != glshader.textures.end(); ++tex_it) {
        GLint loc = tex_it->second;
        glActiveTexture(GL_TEXTURE0 + texUnit);
        CHECK_FOR_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, ctx->LookupTexture(tex_it->first.get()));
        CHECK_FOR_GL_ERROR();
        glUniform1i(loc, texUnit++);
        CHECK_FOR_GL_ERROR();
    }

    map<ICubemapPtr, GLint>::iterator cube_it = glshader.cubemaps.begin();
    for (; cube_it != glshader.cubemaps.end(); ++cube_it) {
        GLint loc = cube_it->second;
        glActiveTexture(GL_TEXTURE0 + texUnit);
        CHECK_FOR_GL_ERROR();
        glBindTexture(GL_TEXTURE_CUBE_MAP, ctx->LookupCubemap(cube_it->first.get()));
        CHECK_FOR_GL_ERROR();
        glUniform1i(loc, texUnit++);
        CHECK_FOR_GL_ERROR();        
    }    
}

void RenderingView::UnbindTextures2D(GLContext::GLShader& glshader) {
    GLint texUnit = 0;
    for (unsigned int i = 0; i < glshader.textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + texUnit);
        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_FOR_GL_ERROR();
        ++texUnit;
    }
    for (unsigned int i = 0; i < glshader.cubemaps.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + texUnit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        CHECK_FOR_GL_ERROR();
        ++texUnit;
    }
}

/**
 * Process a mesh node.
 *
 * @param node Mesh node to render
 */
void RenderingView::VisitMeshNode(MeshNode* node) {
    Mesh* mesh = node->GetMesh().get();
    Material* mat = mesh->GetMaterial().get();
    
    if (mat->transparency > 0.0) {
        RenderObject ro;
        ro.mesh = mesh;
        ro.modelViewMatrix = Matrix<4,4,float>(modelViewMatrix);
        transparencyQueue.push_back(ro);
    }
    else RenderMesh(mesh, modelViewMatrix); 

    node->VisitSubNodes(*this);
    CHECK_FOR_GL_ERROR();
}

void RenderingView::RenderSkybox(const Canvas3D& canvas, GLContext& ctx) {
    glDisable(GL_DEPTH_TEST);
#if FIXED_FUNCTION
    // Do nothing in fixed function, who needs it anyway :)
    // except for the shadersupport check ;-)
if (ctx.ShaderSupport()) {        
#endif
    // Draw skybox to background
    static Shader* skybox = NULL;
    if (skybox == NULL){
        string vert = 
            "uniform mat4 oe_ViewProjMatrixInverse; \n\
            attribute vec3 oe_Vertex; \n                                \
            varying vec3 eyedir; \n                                     \
            void main() { \n                                            \
            gl_Position = gl_Vertex; \n                                 \
            vec4 dir = vec4(gl_Vertex.x, gl_Vertex.y, 1.0, 1.0);\n      \
            eyedir = -normalize((oe_ViewProjMatrixInverse * dir).xyz); \n \
            }";
        
        string frag = 
            "uniform samplerCube skybox; \n\
            varying vec3 eyedir; \n\
            void main() { gl_FragColor = textureCube(skybox, eyedir); }";
        skybox = new Shader(vert, frag);
    }

    GLContext::GLShader glshader= ctx.LookupShader(skybox);
    
    skybox->SetCubemap("skybox", canvas.GetSkybox());
    Matrix<4,4,float> viewProjInv = (canvas.GetViewingVolume()->GetViewMatrix() * 
                                     canvas.GetViewingVolume()->GetProjectionMatrix()).GetInverse();
    skybox->GetUniform("oe_ViewProjMatrixInverse").Set(viewProjInv);

    glUseProgram(glshader.id);
    BindUniforms(glshader);
#ifndef OE_IOS
    glRecti(-1,-1,1,1);
#endif
    glUseProgram(0);
#if FIXED_FUNCTION
 }        
#endif
}

void RenderingView::RenderMesh(Mesh* mesh, Matrix<4,4,float> mvMatrix) {
    // index buffer
    IDataBlock* indices = mesh->indices.get();

    GLsizei count = mesh->GetDrawingRange();
    GLsizei offset = mesh->GetIndexOffset();
    Geometry::Type type = mesh->GetType();

    PhongShader* shad;

    // material
    Material* mat = mesh->GetMaterial().get();    
    if (mat->transparency > 0.0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_CONSTANT_ALPHA, GL_CONSTANT_ALPHA);
        glBlendColor(0.0, 0.0, 0.0, mat->transparency);
        glBlendEquation(GL_FUNC_ADD);
    }
    
#if FIXED_FUNCTION
    if (renderShader && ctx->ShaderSupport()) {        
#endif
        // for now we simply rebind everything in each lookup
        // todo: optimize to only rebind when needed (event driven rebinding).
        // todo: optimize by caching locations
        map<Mesh*, PhongShader*>::iterator it = shaders.find(mesh);
        if (it != shaders.end())
            shad = it->second;
        else {
            shad = new PhongShader(mesh);
            shad->SetLight(light, Vector<4,float>(0.3, 0.3, 0.3, 1.0));
            shaders[mesh] = shad;
        }
        shad->SetModelViewMatrix(mvMatrix);
        shad->SetModelViewProjectionMatrix(mvMatrix * projectionMatrix);

        GLContext::GLShader glshader = ctx->LookupShader(shad);
        glUseProgram(glshader.id);
        BindUniforms(glshader);
        BindAttributes(glshader);
        BindTextures2D(glshader);

        if (ctx->VBOSupport()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->LookupVBO(indices));
            glDrawElements(type, 
                           count, 
                           indices->GetType(), 
                           (GLvoid*)(offset * GLContext::GLTypeSize(indices->GetType())));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        else {
            glDrawElements(type, 
                           count, 
                           indices->GetType(), 
                           (char*)indices->GetVoidDataPtr() + offset * GLContext::GLTypeSize(indices->GetType()));
        }
        UnbindAttributes(glshader);        
        UnbindTextures2D(glshader);        
        glUseProgram(0);
        
#if FIXED_FUNCTION

    } else {
        
        float f[16];
        mvMatrix.ToArray(f);
        glLoadIdentity();
        glMultMatrixf(f);
        CHECK_FOR_GL_ERROR();
   
    GeometrySet* geom = mesh->GetGeometrySet().get();
    
    if (renderTexture && mat->Get2DTextures().size() > 0) {
        glEnable(GL_TEXTURE_2D);
        ITexture2D* tex = (*mat->Get2DTextures().begin()).second.get();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ctx->LookupTexture(tex));
        CHECK_FOR_GL_ERROR();
    }

    // logger.info << "ambient: " << mat->ambient << logger.end;
    // logger.info << "diffuse: " << mat->diffuse << logger.end;
    // logger.info << "specular: " << mat->specular << logger.end;
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  mat->diffuse.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  mat->ambient.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat->emission.ToArray());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
    CHECK_FOR_GL_ERROR();
    
    // Apply the index buffer and draw
    IDataBlock* v     = geom->GetVertices().get();
    IDataBlock* n     = geom->GetNormals().get();
    IDataBlock* c     = geom->GetColors().get();
    IDataBlockList ts = geom->GetTexCoords();

    if (v) glEnableClientState(GL_VERTEX_ARRAY);
    if (n) glEnableClientState(GL_NORMAL_ARRAY);
    if (c) glEnableClientState(GL_COLOR_ARRAY);
    CHECK_FOR_GL_ERROR();

    // enable required texture units
    // @todo: ensure that we do not exceed max number of units
    IDataBlockList::iterator itr = ts.begin();
    unsigned int i = 0;
    for (; itr != ts.end(); ++itr) {
        IDataBlock* t = (*itr).get();
        glClientActiveTexture(GL_TEXTURE0 + i);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        CHECK_FOR_GL_ERROR();
        if (ctx->VBOSupport()) {
            glBindBuffer(GL_ARRAY_BUFFER, ctx->LookupVBO(t));
            glTexCoordPointer(t->GetDimension(), GL_FLOAT, 0, 0);
        }
        else {
            glTexCoordPointer(t->GetDimension(), GL_FLOAT, 0, t->GetVoidDataPtr());
        }
        ++i;
    }

    if (ctx->VBOSupport()) {
        if (v) { 
            glBindBuffer(GL_ARRAY_BUFFER, ctx->LookupVBO(v)); 
            glVertexPointer(v->GetDimension(), GL_FLOAT, 0, 0); 
        }
        if (n) {
            glBindBuffer(GL_ARRAY_BUFFER, ctx->LookupVBO(n));
            glNormalPointer(GL_FLOAT, 0, 0);  
        }
        if (c) { 
            glBindBuffer(GL_ARRAY_BUFFER, ctx->LookupVBO(c));
            glColorPointer(c->GetDimension(), GL_FLOAT, 0, 0); 
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->LookupVBO(indices));
        glDrawElements(type, 
                       count, 
                       indices->GetType(), 
                       (GLvoid*)(offset * GLContext::GLTypeSize(indices->GetType())));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else {
        if (v) glVertexPointer(v->GetDimension(), GL_FLOAT, 0, v->GetVoidDataPtr());
        if (n) glNormalPointer(GL_FLOAT, 0, n->GetVoidDataPtr());
        if (c) glColorPointer(c->GetDimension(), GL_FLOAT, 0, c->GetVoidDataPtr());
        glDrawElements(type, 
                       count, 
                       indices->GetType(), 
                       (char*)indices->GetVoidDataPtr() + offset * GLContext::GLTypeSize(indices->GetType()));
    }

    // cleanup
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    itr = ts.begin();
    i = 0;
    for (; itr != ts.end(); ++itr) {
        glClientActiveTexture(GL_TEXTURE0 + i);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        ++i;
    }
    CHECK_FOR_GL_ERROR();
    }
#endif
    glDisable(GL_BLEND);
}

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine
