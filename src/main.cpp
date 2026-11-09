#include "CApplication.h"
#include "CWindowsFont.h"

#include "imgui.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_win32.h"
#include "implot.h"
#include "theme/spectrum.h"

#include <windows.h>
#include <gl/GL.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

static LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
  if (ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
    return 1;

  if (Message == WM_SIZE && WParam != SIZE_MINIMIZED)
    return 0;
  if (Message == WM_SYSCOMMAND && (WParam & 0xfff0) == SC_KEYMENU)
    return 0;
  if (Message == WM_DESTROY)
  {
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(Window, Message, WParam, LParam);
}

static bool CreateOpenGLContext(HWND Window, HDC& DeviceContext, HGLRC& RenderingContext)
{
  DeviceContext = GetDC(Window);
  PIXELFORMATDESCRIPTOR PixelFormat = {};
  PixelFormat.nSize = sizeof(PixelFormat);
  PixelFormat.nVersion = 1;
  PixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  PixelFormat.iPixelType = PFD_TYPE_RGBA;
  PixelFormat.cColorBits = 32;
  PixelFormat.cDepthBits = 24;
  PixelFormat.iLayerType = PFD_MAIN_PLANE;
  int Format = ChoosePixelFormat(DeviceContext, &PixelFormat);
  if (Format == 0 || !SetPixelFormat(DeviceContext, Format, &PixelFormat))
    return false;

  RenderingContext = wglCreateContext(DeviceContext);
  return RenderingContext != NULL && wglMakeCurrent(DeviceContext, RenderingContext) == TRUE;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE, LPSTR, int ShowCommand)
{
  ImGui_ImplWin32_EnableDpiAwareness();

  const wchar_t* ClassName = L"DevTermWindow";
  WNDCLASSEXW WindowClass = {};
  WindowClass.cbSize = sizeof(WindowClass);
  WindowClass.style = CS_CLASSDC;
  WindowClass.lpfnWndProc = WindowProcedure;
  WindowClass.hInstance = Instance;
  WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  WindowClass.lpszClassName = ClassName;
  if (!RegisterClassExW(&WindowClass))
  {
    MessageBoxW(NULL, L"Das Anwendungsfenster konnte nicht registriert werden.",
                L"DevTerm", MB_OK | MB_ICONERROR);
    return 1;
  }

  HWND Window = CreateWindowW(ClassName, L"DevTerm - SCPI Terminal",
                              WS_OVERLAPPEDWINDOW, 100, 100, 1100, 720,
                              NULL, NULL, Instance, NULL);
  if (Window == NULL)
  {
    MessageBoxW(NULL, L"Das Anwendungsfenster konnte nicht erstellt werden.",
                L"DevTerm", MB_OK | MB_ICONERROR);
    UnregisterClassW(ClassName, Instance);
    return 1;
  }

  HDC DeviceContext = NULL;
  HGLRC RenderingContext = NULL;
  if (!CreateOpenGLContext(Window, DeviceContext, RenderingContext))
  {
    if (RenderingContext != NULL)
      wglDeleteContext(RenderingContext);
    if (DeviceContext != NULL)
      ReleaseDC(Window, DeviceContext);
    DestroyWindow(Window);
    UnregisterClassW(ClassName, Instance);
    MessageBoxW(NULL, L"Der OpenGL-Kontext konnte nicht initialisiert werden.",
                L"DevTerm", MB_OK | MB_ICONERROR);
    return 1;
  }

  ShowWindow(Window, ShowCommand);
  UpdateWindow(Window);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGui::Spectrum::StyleColorsSpectrum(false);
  ImPlot::StyleColorsAuto();
  ImGui::GetIO().IniFilename = NULL;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  if (!LoadWindowsDefaultFont(16.0f))
  {
    MessageBoxW(Window, L"Segoe UI konnte nicht geladen werden. Der ImGui-Standardfont wird verwendet.",
                L"DevTerm", MB_OK | MB_ICONWARNING);
    ImGui::Spectrum::LoadFont(13.0f);
  }
  ImGui_ImplWin32_Init(Window);
  ImGui_ImplOpenGL2_Init();

  int ExitCode = 0;
  bool Running = true;
  {
    CApplication Application;
    while (Running)
    {
      MSG Message;
      while (PeekMessageW(&Message, NULL, 0, 0, PM_REMOVE))
      {
        if (Message.message == WM_QUIT)
        {
          ExitCode = static_cast<int>(Message.wParam);
          Running = false;
          break;
        }
        TranslateMessage(&Message);
        DispatchMessageW(&Message);
      }
      if (!Running)
        break;

      ImGui_ImplOpenGL2_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();
      Application.Render();
      ImGui::Render();

      RECT ClientArea;
      GetClientRect(Window, &ClientArea);
      glViewport(0, 0, ClientArea.right - ClientArea.left, ClientArea.bottom - ClientArea.top);
      const ImVec4& Background = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
      glClearColor(Background.x, Background.y, Background.z, Background.w);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
      SwapBuffers(DeviceContext);
      Sleep(10);
    }
  }

  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(RenderingContext);
  ReleaseDC(Window, DeviceContext);
  DestroyWindow(Window);
  UnregisterClassW(ClassName, Instance);
  return ExitCode;
}
