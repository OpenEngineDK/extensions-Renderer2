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

ShadowMap::DepthRenderer::DepthRenderer(unsigned int width, unsigned int height)
  : width(width) 
  , height(height)
  , canvas(new Canvas3D(width,height))
{
    
}

void ShadowMap::DepthRenderer::Initialize(GLContext* ctx, Shader* shader) {
    if (!ctx->FBOSupport()) {
        throw Exception("Shadowmap does not work without FBOSupport.");
    }
    GLContext::Attachments atts = ctx->LookupCanvas(canvas);
    ctx->LookupFBO(canvas);

    shader->SetTexture2D("shadow", atts.depth);
}

void ShadowMap::DepthRenderer::Render(ISceneNode* scene, IViewingVolume& cam, GLContext* ctx) {
    modelViewMatrix = cam.GetViewMatrix();
    projectionMatrix = cam.GetProjectionMatrix();

    GLint prevFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

    GLuint fbo = ctx->LookupFBO(canvas);
    // Setup the new frame buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    CHECK_FOR_GL_ERROR();

    // Turn off unneeded stuff!
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(2.1, 4.0);



    // Select The Projection Matrix
    glMatrixMode(GL_PROJECTION);
    CHECK_FOR_GL_ERROR();

    // Reset The Projection Matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Setup OpenGL with the volumes projection matrix
    Matrix<4,4,float> projMatrix = cam.GetProjectionMatrix();
    float arr[16] = {0};
    projMatrix.ToArray(arr);
    glMultMatrixf(arr);
    CHECK_FOR_GL_ERROR();

    // Select the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    CHECK_FOR_GL_ERROR();

    // Reset the modelview matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Get the view matrix and apply it
    Matrix<4,4,float> matrix = cam.GetViewMatrix();
    float f[16] = {0};
    matrix.ToArray(f);
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();



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
    float f[16];
    m.ToArray(f);
    glPushMatrix();
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
    node->VisitSubNodes(*this);
    glPopMatrix();
    CHECK_FOR_GL_ERROR();


    // Matrix<4,4,float> m = node->GetTransformationMatrix();
    // Matrix<4,4,float> oldMv = modelViewMatrix;
    // modelViewMatrix = m * modelViewMatrix;
    // node->VisitSubNodes(*this);
    // modelViewMatrix = oldMv;
}

void ShadowMap::DepthRenderer::VisitMeshNode(MeshNode* node) {
    MeshPtr mesh = node->GetMesh();
    GeometrySetPtr geom = mesh->GetGeometrySet();

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    IDataBlockPtr v = geom->GetVertices();

    glBindBuffer(GL_ARRAY_BUFFER, v->GetID());
    if (v->GetID() != 0)
        glVertexPointer(v->GetDimension(), GL_FLOAT, 0, 0);
    else
        glVertexPointer(v->GetDimension(), GL_FLOAT, 0, v->GetVoidDataPtr());


    CHECK_FOR_GL_ERROR();

    IndicesPtr indexBuffer = mesh->GetIndices();
    GLsizei count = mesh->GetDrawingRange();
    unsigned int offset = mesh->GetIndexOffset();
    Geometry::Type type = mesh->GetType();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->GetID());
    if (indexBuffer->GetID() != 0){
        glDrawElements(type, count, GL_UNSIGNED_INT, (GLvoid*)(offset * sizeof(GLuint)));
    }
    else {
        glDrawElements(type, count, GL_UNSIGNED_INT, indexBuffer->GetData() + offset);
    }



    node->VisitSubNodes(*this);
    CHECK_FOR_GL_ERROR();
}

ShadowMap::ShadowMap(unsigned int width, unsigned int height)
  : depthRenderer(width, height)
  , shader(ResourceManager<ShaderResource>::Create("shaders/shadowmap.glsl"))
{
    shader->Load();
}

void ShadowMap::SetViewingVolume(IViewingVolume* v) {
    viewingVolume = v;
}

void ShadowMap::Handle(RenderingEventArg arg) {
    if (arg.renderer.GetCurrentStage() == GLRenderer::RENDERER_INITIALIZE) {
        depthRenderer.Initialize(arg.renderer.GetContext(), shader.get());
        
    }
    else if (arg.renderer.GetCurrentStage() == GLRenderer::RENDERER_PREPROCESS) {
        depthRenderer.Render(arg.canvas->GetScene(), *viewingVolume, arg.renderer.GetContext());

        const Matrix<4,4,float> bias(.5, .0, .0,  .0,
                                     .0, .5, .0,  .0,
                                     .0, .0, .5,  .0,
                                     .5, .5, .5, 1.0);

        shader->GetUniform("lightMat").Set(viewingVolume->GetViewMatrix() *
                                           viewingVolume->GetProjectionMatrix() * 
                                           bias);

        shader->GetUniform("viewProjectionInverse").Set((arg.canvas->GetViewingVolume()->GetViewMatrix() * 
                                                         arg.canvas->GetViewingVolume()->GetProjectionMatrix()).GetInverse());
    }
    else {
        GLContext::Attachments atts = arg.renderer.GetContext()->LookupCanvas(arg.canvas);
        shader->SetTexture2D("color0", atts.color0);
        shader->SetTexture2D("depth", atts.depth);

            
        const float verts[8] = {
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f
        };

        
        glDisable(GL_DEPTH_TEST);
        // do the quading with the post process shader
        GLuint shaderId = arg.renderer.Apply(shader.get());
        CHECK_FOR_GL_ERROR();

        const GLint vsLoc = glGetAttribLocation(shaderId, "vertex");
        glEnableVertexAttribArray(vsLoc);
        glVertexAttribPointer(vsLoc, 2, GL_FLOAT, GL_FALSE, 0, verts);
        CHECK_FOR_GL_ERROR();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        CHECK_FOR_GL_ERROR();
        arg.renderer.Release(shader.get());        
        glDisableVertexAttribArray(vsLoc);

        glEnable(GL_DEPTH_TEST);
    } 
}



}
}
}

