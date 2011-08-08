// OpenGL renderer implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers2/OpenGL/GLRenderer.h>

#include <Renderers2/OpenGL/RenderingView.h>
#include <Renderers2/OpenGL/LightVisitor.h>
#include <Renderers2/OpenGL/CanvasVisitor.h>
#include <Resources/ResourceManager.h>
#include <Resources2/ShaderResource.h>

#include <Renderers2/OpenGL/GLContext.h>
#include <Display2/Canvas3D.h>
#include <Display2/CompositeCanvas.h>
#include <Display/IViewingVolume.h>
#include <Meta/OpenGL.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers2 {
namespace OpenGL {

using Display::IViewingVolume;

using namespace Display2;
using Resources::ResourceManager;
using Resources::DirectoryManager;
using Resources2::ShaderResource;
using Resources2::ShaderResourcePtr;
using namespace Math;

GLRenderer::GLRenderer(GLContext* ctx)
    : ctx(ctx)
    , canvas(NULL)
    , init(false)
    , level(0)
    , rv(new RenderingView())
    , lv(new LightVisitor())
    , cv(new CanvasVisitor(*this))
    , arg(Core::ProcessEventArg(Time(), 0))
    , stage(RENDERER_UNINITIALIZE)
{
    DirectoryManager::AppendPath("extensions/Renderer2/");
    quadShader = ResourceManager<ShaderResource>::Create("shaders/QuadShader.glsl");
    quadShader->Load();
    
    preProcess.Attach(*lv);
    process.Attach(*rv);
}
    
GLRenderer::~GLRenderer() {

}

void GLRenderer::Render(CompositeCanvas* canvas) {
    // logger.info << "render composite: " << canvas << logger.end;
    ++level;
    canvas->AcceptChildren(*cv);
    --level;

    GLint prevFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

    if (ctx->FBOSupport() && level > 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, ctx->LookupFBO(canvas));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx->LookupCanvas(canvas), 0);
        CHECK_FRAMEBUFFER_STATUS();
   }

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glActiveTexture(GL_TEXTURE0);

    glViewport(0, 0, canvas->GetWidth(), canvas->GetHeight());
    RGBAColor bgc = canvas->GetBackgroundColor();
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    const float texc[8] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };

#if FIXED_FUNCTION
    if (ctx->ShaderSupport()) {
#endif
        GLContext::GLShader glShader = ctx->LookupShader(quadShader.get());
        GLuint shaderId = glShader.id;
        glUseProgram(shaderId);
        
        const GLint vsLoc = glGetAttribLocation(shaderId, "vertex");
        const GLint tcLoc = glGetAttribLocation(shaderId, "tcIn");
        const GLint clLoc = glGetUniformLocation(shaderId, "color");
        const GLint txLoc = glGetUniformLocation(shaderId, "texIn");
        const GLint dimLoc = glGetUniformLocation(shaderId, "dims");
                
        glEnableVertexAttribArray(vsLoc);
        glEnableVertexAttribArray(tcLoc);
        CHECK_FOR_GL_ERROR();
 
        glVertexAttribPointer(tcLoc, 2, GL_FLOAT, GL_FALSE, 0, texc);
        CHECK_FOR_GL_ERROR();

        glUniform2f(dimLoc, (float)canvas->GetWidth(), (float)canvas->GetHeight());
        CHECK_FOR_GL_ERROR();

        CompositeCanvas::ContainerIterator it = canvas->CanvasesBegin();
        for (; it != canvas->CanvasesEnd(); ++it) { 
            glColorMask(it->redMask, it->greenMask, it->blueMask, it->alphaMask);
            const float w = it->w;
            const float h = it->h;
            const float x = it->x;
            const float y = canvas->GetHeight() - it->y;
            const float vert[8] = {
                x, y, 
                x, y - h, 
                x + w, y,
                x + w, y - h
            };
            
            float col[4];
            it->color.ToArray(col);
            col[3] = it->opacity;
            glUniform4fv(clLoc, 1, col);
            CHECK_FOR_GL_ERROR();

            glBindTexture(GL_TEXTURE_2D, ctx->LookupCanvas(it->canvas));
            glUniform1i(txLoc, 0);
            CHECK_FOR_GL_ERROR();

            glVertexAttribPointer(vsLoc, 2, GL_FLOAT, GL_FALSE, 0, vert);            
            CHECK_FOR_GL_ERROR();

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            CHECK_FOR_GL_ERROR();
        }

        glUseProgram(0);
        glDisableVertexAttribArray(vsLoc);
        glDisableVertexAttribArray(tcLoc);

#if FIXED_FUNCTION
    }
    else {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, canvas->GetWidth(), 0.0, canvas->GetHeight(), 0.0, 1.0);
        CHECK_FOR_GL_ERROR();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, texc);
        CHECK_FOR_GL_ERROR();

        CompositeCanvas::ContainerIterator it = canvas->CanvasesBegin();
        for (; it != canvas->CanvasesEnd(); ++it) { 
            glColorMask(it->redMask, it->greenMask, it->blueMask, it->alphaMask);
            const float w = it->w;
            const float h = it->h;
            const float x = it->x;
            const float y = canvas->GetHeight() - it->y;
            const float vert[8] = {
                x, y, 
                x, y - h, 
                x + w, y,
                x + w, y - h
            };
            glVertexPointer(2, GL_FLOAT, 0, vert);
        

            float col[16];
            it->color.ToArray(col);
            it->color.ToArray(&col[4]);
            it->color.ToArray(&col[8]);
            it->color.ToArray(&col[12]);
            col[3] = col[7] = col[11] = col[15] =  it->opacity;
            glColorPointer(4, GL_FLOAT, 0, col);

            glBindTexture(GL_TEXTURE_2D, ctx->LookupCanvas(it->canvas));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            CHECK_FOR_GL_ERROR();
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
    }
#endif

    if (ctx->FBOSupport() && level > 0) {
        //bind the previous back buffer again
        glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, ctx->LookupCanvas(canvas));
        CHECK_FOR_GL_ERROR();
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GLContext::GLInternalColorFormat(canvas->GetColorFormat()), 
                         0, 0, canvas->GetWidth(), canvas->GetHeight(), 0);
        CHECK_FOR_GL_ERROR();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_BLEND);
}

void GLRenderer::Render(Canvas3D* canvas) {
    GLint prevFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

    if (ctx->FBOSupport() && level > 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, ctx->LookupFBO(canvas));
    }

    // logger.info << "render c3d: " << canvas << logger.end;
    // @todo: assert we are in preprocess stage
    RGBAColor bgc = canvas->GetBackgroundColor();
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the screen and the depth buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    IViewingVolume* volume = canvas->GetViewingVolume();
    // If no viewing volume is set for the viewport ignore it.
    if (volume != NULL) {
        volume->SignalRendering(arg.approx);

        // Set viewport size 
        Vector<4,int> d(0, 0, canvas->GetWidth(), canvas->GetHeight());
        glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
        CHECK_FOR_GL_ERROR();

        // apply the volume
        ApplyViewingVolume(*volume);
    }
    CHECK_FOR_GL_ERROR();

    // run the processing phases
    RenderingEventArg rarg(canvas, *this, arg.start, arg.approx);
    this->preProcess.Notify(rarg);
    
#if OE_SAFE
    if (lv->GetLights().empty()) throw Exception("No lights in scene. PhongShader will be confused...");
#endif
    rv->light = lv->GetLights().at(0);
    
    this->stage = RENDERER_PROCESS;
    this->process.Notify(rarg);
    this->stage = RENDERER_POSTPROCESS;
    this->postProcess.Notify(rarg);
    this->stage = RENDERER_PREPROCESS;

    if (ctx->FBOSupport() && level > 0) {
        //bind the previous back buffer again
        glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    }
    else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ctx->LookupCanvas((ICanvas*)canvas));
        CHECK_FOR_GL_ERROR();
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GLContext::GLInternalColorFormat(canvas->GetColorFormat()), 
                         0, 0, canvas->GetWidth(), canvas->GetHeight(), 0);
        CHECK_FOR_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void GLRenderer::Handle(Core::InitializeEventArg arg) {
    ctx->Init();
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);						   
    CHECK_FOR_GL_ERROR();

#if FIXED_FUNCTION
    // Set perspective calculations to most accurate
#ifndef OE_IOS
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#endif

    glShadeModel(GL_SMOOTH);
    CHECK_FOR_GL_ERROR();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    CHECK_FOR_GL_ERROR();
    GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
#endif
    this->stage = RENDERER_INITIALIZE;
    this->initialize.Notify(RenderingEventArg(NULL, *this));
    this->stage = RENDERER_PREPROCESS;
    CHECK_FOR_GL_ERROR();

    // traverse scene graph to load gpu resource.
    canvas->Accept(*cv);

    init = true;
}
    
void GLRenderer::Handle(Core::DeinitializeEventArg arg) {

}
    
void GLRenderer::Handle(Core::ProcessEventArg arg) {
    this->arg = arg;
    canvas->Accept(*cv);

}

IEvent<RenderingEventArg>& GLRenderer::InitializeEvent() {
    return initialize;
}

IEvent<RenderingEventArg>& GLRenderer::PreProcessEvent() {
    return preProcess;
}
    
IEvent<RenderingEventArg>& GLRenderer::ProcessEvent() {
    return process;
}

IEvent<RenderingEventArg>& GLRenderer::PostProcessEvent() {
    return postProcess;
}
    
IEvent<RenderingEventArg>& GLRenderer::DeinitializeEvent() {
    return deinitialize;
}

void GLRenderer::SetCanvas(ICanvas* canvas) {
    this->canvas = canvas;
}

ICanvas* GLRenderer::GetCanvas() {
    return canvas;
}

void GLRenderer::ApplyViewingVolume(IViewingVolume& volume) {
// @todo: consider moving this to the rendering view.
#if FIXED_FUNCTION
    // Select The Projection Matrix
    glMatrixMode(GL_PROJECTION);
    CHECK_FOR_GL_ERROR();

    // Reset The Projection Matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Setup OpenGL with the volumes projection matrix
    Matrix<4,4,float> projMatrix = volume.GetProjectionMatrix();
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
    Matrix<4,4,float> matrix = volume.GetViewMatrix();
    float f[16] = {0};
    matrix.ToArray(f);
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
#endif
}

GLContext* GLRenderer::GetContext() {
    return ctx;
}




void GLRenderer::BindUniforms(GLContext::GLShader& glshader) {
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


void GLRenderer::BindAttributes(GLContext::GLShader& glshader) {
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

void GLRenderer::UnbindAttributes(GLContext::GLShader& glshader) {
    map<IDataBlockPtr, GLint>::iterator it = glshader.attributes.begin();
    for (; it != glshader.attributes.end(); ++it) {
        glDisableVertexAttribArray(it->second);
        CHECK_FOR_GL_ERROR();
    }
}

void GLRenderer::BindTextures2D(GLContext::GLShader& glshader) {
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

void GLRenderer::UnbindTextures2D(GLContext::GLShader& glshader) {
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


GLuint GLRenderer::Apply(Shader* shader) {
    GLContext::GLShader glshader = ctx->LookupShader(shader);
    glUseProgram(glshader.id);
    BindUniforms(glshader);
    BindAttributes(glshader);
    BindTextures2D(glshader);
    return glshader.id;
}

void GLRenderer::Release(Shader* shader) {
    GLContext::GLShader glshader = ctx->LookupShader(shader);
    UnbindAttributes(glshader);        
    UnbindTextures2D(glshader);        
    glUseProgram(0);
}


// void GLRenderer::Render(Shader* shader, Canvas3D* canvas) {
//     GLint prevFBO;

//     if (canvas && ctx->FBOSupport()) {
//         glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
//         glBindFramebuffer(GL_FRAMEBUFFER, ctx->LookupFBO(canvas));
//     }


//     if (canvas && ctx->FBOSupport()) {
//         glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
//     }
// } 

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine
