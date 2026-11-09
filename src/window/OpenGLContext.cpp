#include "OpenGLContext.h"

#include <gl/GL.h>

COpenGLContext::COpenGLContext()
  : m_Window(NULL),
    m_DeviceContext(NULL),
    m_RenderingContext(NULL)
{
}

COpenGLContext::~COpenGLContext()
{
  Destroy();
}

bool COpenGLContext::Create(HWND Window)
{
  m_Window = Window;
  m_DeviceContext = GetDC(Window);
  if (m_DeviceContext == NULL)
  {
    m_Window = NULL;
    return false;
  }

  PIXELFORMATDESCRIPTOR PixelFormat = {};
  PixelFormat.nSize = sizeof(PixelFormat);
  PixelFormat.nVersion = 1;
  PixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                        PFD_DOUBLEBUFFER;
  PixelFormat.iPixelType = PFD_TYPE_RGBA;
  PixelFormat.cColorBits = 32;
  PixelFormat.cAlphaBits = 8;
  PixelFormat.cDepthBits = 24;
  PixelFormat.iLayerType = PFD_MAIN_PLANE;

  int Format = ChoosePixelFormat(m_DeviceContext, &PixelFormat);
  if (Format == 0 || !SetPixelFormat(m_DeviceContext, Format, &PixelFormat))
  {
    Destroy();
    return false;
  }

  m_RenderingContext = wglCreateContext(m_DeviceContext);
  if (m_RenderingContext == NULL ||
      !wglMakeCurrent(m_DeviceContext, m_RenderingContext))
  {
    Destroy();
    return false;
  }
  return true;
}

void COpenGLContext::Present()
{
  if (m_DeviceContext != NULL)
    SwapBuffers(m_DeviceContext);
}

void COpenGLContext::Destroy()
{
  if (m_RenderingContext != NULL &&
      wglGetCurrentContext() == m_RenderingContext)
    wglMakeCurrent(NULL, NULL);
  if (m_RenderingContext != NULL)
  {
    wglDeleteContext(m_RenderingContext);
    m_RenderingContext = NULL;
  }
  if (m_DeviceContext != NULL)
  {
    ReleaseDC(m_Window, m_DeviceContext);
    m_DeviceContext = NULL;
  }
  m_Window = NULL;
}
