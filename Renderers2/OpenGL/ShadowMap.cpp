// Shadow map rendering effect.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include <Renderers2/OpenGL/ShadowMap.h>
#include <Renderers2/OpenGL/GLContext.h>
#include <Display2/Canvas3D.h>
#include <Display/IViewingVolume.h>
#include <Scene/TransformationNode.h>
#include <Scene/MeshNode.h>
#include <Logging/Logger.h>
#include <Geometry/Mesh.h>
#include <Geometry/GeometrySet.h>
#include <Resources/ResourceManager.h>
#include <Resources2/ShaderResource.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using namespace Resources;
using namespace Math;
using namespace Display;
using namespace Geometry;

using Display2::Canvas3D;
using Resources2::ShaderResource;

    string vert = 
        "   uniform mat4 modelViewProjectionMatrix; \n                  \
            attribute vec3 vertex;  \n                                  \
            void main() { \n                                            \
              gl_Position = modelViewProjectionMatrix * vec4(vertex, 1.0); \n \
            }";
        
    string frag = " void main() { }";
    
ShadowMap::DepthRenderer::DepthRenderer(unsigned int width, unsigned int height)
  : width(width) 
  , height(height)
  , canvas(new Canvas3D(width,height))
  , shader(new Shader(vert, frag))
  , mvpUniform(shader->GetUniform("modelViewProjectionMatrix"))
  , vertAttrib(shader->GetAttribute("vertex"))
  , ctx(NULL)
  , renderer(NULL)
  , num1(2.1)
  , num2(4.0)
{
}

void ShadowMap::DepthRenderer::Initialize(GLContext* ctx, Shader* shader) {
    if (!ctx->FBOSupport()) {
        throw Exception("Shadowmap does not work without FBOSupport.");
    }
    GLContext::Attachments atts = ctx->LookupCanvas(canvas);
    shader->GetTexture2D("shadow").Set(atts.depth);
}

void ShadowMap::DepthRenderer::Render(ISceneNode* scene, IViewingVolume& cam, GLRenderer* renderer) {
    this->renderer = renderer;
    ctx = renderer->GetContext();
    modelViewMatrix = cam.GetViewMatrix();
    projectionMatrix = cam.GetProjectionMatrix();

    GLint prevFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

    GLuint fbo = ctx->LookupFBO(canvas);

    // Setup the new frame buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                           ctx->LookupTexture(ctx->LookupCanvas(canvas).color0.get()), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
                           ctx->LookupTexture(ctx->LookupCanvas(canvas).depth.get()), 0);

    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    CHECK_FOR_GL_ERROR();

    // Turn off unneeded stuff!
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(num1, num2);

    // Draw it
    scene->Accept(*this);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glCullFace(GL_BACK);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    CHECK_FOR_GL_ERROR();
}

void ShadowMap::DepthRenderer::VisitTransformationNode(TransformationNode* node) {
    Matrix<4,4,float> m = node->GetTransformationMatrix();
    Matrix<4,4,float> oldMv = modelViewMatrix;
    modelViewMatrix = m * modelViewMatrix;
    node->VisitSubNodes(*this);
    modelViewMatrix = oldMv;
}

void ShadowMap::DepthRenderer::VisitMeshNode(MeshNode* node) {
    MeshPtr mesh = node->GetMesh();
    GeometrySetPtr geom = mesh->GetGeometrySet();

    vertAttrib.Set(geom->GetVertices());
    mvpUniform.Set(modelViewMatrix * projectionMatrix);

    IDataBlock* indices = mesh->indices.get();
    GLsizei count = mesh->GetDrawingRange();
    unsigned int offset = mesh->GetIndexOffset();
    Geometry::Type type = mesh->GetType();

    renderer->Apply(shader);
    
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
    CHECK_FOR_GL_ERROR();

    renderer->Release(shader);
    
    node->VisitSubNodes(*this);
}

ShadowMap::ShadowMap(unsigned int width, unsigned int height)
  : depthRenderer(width, height)
  , shader(ResourceManager<ShaderResource>::Create("shaders/shadowmap.glsl"))
  , lightMatrix(shader->GetUniform("lightMat"))
  , viewProjectionInverse(shader->GetUniform("viewProjectionInverse"))
  , active(true)
{
    shader->Load();
}

void ShadowMap::SetViewingVolume(IViewingVolume* v) {
    viewingVolume = v;
}

void ShadowMap::Handle(RenderingEventArg arg) {
    GLContext* ctx = arg.renderer.GetContext();
    // this is a hack! Module should not be added in the first place if shader is not supported.
    if (!ctx->ShaderSupport()) return; 
    if (arg.renderer.GetCurrentStage() == GLRenderer::RENDERER_INITIALIZE) {
        depthRenderer.Initialize(ctx, shader.get());
        
        const float verts[8] = {
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f
        };
        DataBlock<2,float>* db = new DataBlock<2,float>(4);
        memcpy(db->GetVoidDataPtr(), verts, 4 * 2 * sizeof(float));
        shader->GetAttribute("vertex").Set(IDataBlockPtr(db));        
    }
    // else if (arg.renderer.GetCurrentStage() == GLRenderer::RENDERER_PREPROCESS) {
    // }
    else {           
        if (!active) return;
        depthRenderer.Render(arg.canvas->GetScene(), *viewingVolume, &arg.renderer);
        
        const Matrix<4,4,float> bias(.5, .0, .0,  .0,
                                     .0, .5, .0,  .0,
                                     .0, .0, .5,  .0,
                                     .5, .5, .5, 1.0);
        
        lightMatrix.Set(viewingVolume->GetViewMatrix() *
                        viewingVolume->GetProjectionMatrix() * 
                        bias
                        );
        
        viewProjectionInverse.Set((arg.canvas->GetViewingVolume()->GetViewMatrix() * 
                                   arg.canvas->GetViewingVolume()->GetProjectionMatrix()).GetInverse());


        GLContext::Attachments& atts = arg.renderer.GetContext()->LookupCanvas(arg.canvas);
        shader->GetTexture2D("color0").Set(atts.color0);
        shader->GetTexture2D("depth").Set(atts.depth);
        
        GLint prevFbo; 
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
        GLint fbo = ctx->LookupFBO(arg.canvas);
        
        if (prevFbo == fbo) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                                   ctx->LookupTexture(atts.color1.get()), 0);
            //flip output buffers.
            ITexture2DPtr tmp = atts.color0;
            atts.color0 = atts.color1;
            atts.color1 = tmp;
        }

        glViewport(0, 0, arg.canvas->GetWidth(), arg.canvas->GetHeight());
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        // do the quading with the post process shader
        arg.renderer.Apply(shader.get());
        CHECK_FOR_GL_ERROR();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        CHECK_FOR_GL_ERROR();
        arg.renderer.Release(shader.get());
        CHECK_FOR_GL_ERROR();
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    } 
}

void ShadowMap::SetMagicNumber1(float num) {
    depthRenderer.num1 = num;
}

void ShadowMap::SetMagicNumber2(float num) {
    depthRenderer.num2 = num;
}

}
}
}

