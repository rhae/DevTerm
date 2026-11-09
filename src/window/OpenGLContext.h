#pragma once

#include <windows.h>

class COpenGLContext
{
public:
  COpenGLContext();
  ~COpenGLContext();

  bool Create(HWND Window);
  void Present();
  void Destroy();

private:
  HWND m_Window;
  HDC m_DeviceContext;
  HGLRC m_RenderingContext;
};
