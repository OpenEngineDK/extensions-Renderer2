
# Create the extension library
ADD_LIBRARY(Extensions_Renderers2
  Renderers2/OpenGL/GLRenderer.h
  Renderers2/OpenGL/GLRenderer.cpp
  Renderers2/OpenGL/RenderingView.h
  Renderers2/OpenGL/RenderingView.cpp
  Renderers2/OpenGL/LightVisitor.h
  Renderers2/OpenGL/LightVisitor.cpp
  Renderers2/OpenGL/GLContext.h
  Renderers2/OpenGL/GLContext.cpp
  Resources2/Shader.h
  Resources2/Shader.cpp
  Resources2/PhongShader.h
  Resources2/PhongShader.cpp
  Display2/ICanvas.h
  Display2/Canvas3D.h
)
