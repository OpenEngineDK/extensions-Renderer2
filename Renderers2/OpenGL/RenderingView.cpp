// OpenGL rendering view.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers2/OpenGL/RenderingView.h>

#include <Renderers2/OpenGL/GLContext.h>
#include <Display2/Canvas3D.h>
#include <Scene/TransformationNode.h>
#include <Scene/RenderStateNode.h>
#include <Scene/MeshNode.h>
#include <Geometry/GeometrySet.h>
#include <Geometry/Mesh.h>
#include <Geometry/Material.h>
#include <Meta/OpenGL.h>
#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using namespace Math;
using namespace Geometry;
using namespace Scene;

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
    if (arg.canvas.GetScene() == NULL) 
        throw Exception("Scene was NULL while rendering.");
#endif
    ctx = arg.renderer.GetContext();
    // setup default render state
    ApplyRenderState(currentRenderState);
    arg.canvas.GetScene()->Accept(*this);
    ctx = NULL;
}
            
void RenderingView::ApplyRenderState(RenderStateNode* node) {
    if (node->IsOptionEnabled(RenderStateNode::WIREFRAME)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::WIREFRAME)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

    if (node->IsOptionEnabled(RenderStateNode::LIGHTING)) {
        glEnable(GL_LIGHTING);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::LIGHTING)) {
        glDisable(GL_LIGHTING);
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
    float f[16];
    m.ToArray(f);
    glPushMatrix();
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
    node->VisitSubNodes(*this);
    CHECK_FOR_GL_ERROR();
    glPopMatrix();
    CHECK_FOR_GL_ERROR();
}

/**
 * Process a mesh node.
 *
 * @param node Mesh node to render
 */
void RenderingView::VisitMeshNode(MeshNode* node) {
    Mesh* mesh = node->GetMesh().get();
    GeometrySet* geom = mesh->GetGeometrySet().get();
    Material* mat = mesh->GetMaterial().get();

    // material
    if (renderTexture && mat->Get2DTextures().size() > 0) {
        glEnable(GL_TEXTURE_2D);
        ITexture2D* tex = (*mat->Get2DTextures().begin()).second.get();
        glBindTexture(GL_TEXTURE_2D, ctx->LookupTexture(tex));
        CHECK_FOR_GL_ERROR();
    }
  
    logger.info << "ambient: " << mat->ambient << logger.end;
    logger.info << "diffuse: " << mat->diffuse << logger.end;
    logger.info << "specular: " << mat->specular << logger.end;
    
    mat->ambient = Vector<4,float>(1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  mat->diffuse.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  mat->ambient.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat->emission.ToArray());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
    CHECK_FOR_GL_ERROR();
    
    // Apply the index buffer and draw
    Indices* indices = mesh->GetIndices().get();
    GLsizei count = mesh->GetDrawingRange();
    GLsizei offset = mesh->GetIndexOffset();
    Geometry::Type type = mesh->GetType();

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
        glDrawElements(type, count, GL_UNSIGNED_INT, (GLvoid*)(offset * sizeof(GLuint)));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else {
        if (v) glVertexPointer(v->GetDimension(), GL_FLOAT, 0, v->GetVoidDataPtr());
        if (n) glNormalPointer(GL_FLOAT, 0, n->GetVoidDataPtr());
        if (c) glColorPointer(c->GetDimension(), GL_FLOAT, 0, c->GetVoidDataPtr());
        glDrawElements(type, count, GL_UNSIGNED_INT, indices->GetData() + offset);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
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

    node->VisitSubNodes(*this);
}

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine
