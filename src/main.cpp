#include "Application.h"
#include "WindowsFont.h"
#include "window/OpenGLContext.h"
#include "window/Window.h"
#include "window/Win32Window.h"

#include "imgui.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_win32.h"
#include "implot.h"
#include "theme/spectrum.h"

#include <windows.h>
#include <gl/GL.h>
#include <memory>

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE, LPSTR, int ShowCommand)
{
  ImGui_ImplWin32_EnableDpiAwareness();

  std::unique_ptr<CWindow> Window(new CWin32Window());
  if (!Window->Create(Instance, L"DevTerm - SCPI Terminal", 1100, 720))
  {
    MessageBoxW(NULL, L"Das Anwendungsfenster konnte nicht erstellt werden.",
                L"DevTerm", MB_OK | MB_ICONERROR);
    return 1;
  }

  COpenGLContext RenderingContext;
  if (!RenderingContext.Create(Window->GetHandle()))
  {
    MessageBoxW(Window->GetHandle(),
                L"Der OpenGL-Kontext konnte nicht initialisiert werden.",
                L"DevTerm", MB_OK | MB_ICONERROR);
    return 1;
  }

  Window->Show(ShowCommand);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGui::Spectrum::StyleColorsSpectrum(false);
  ImPlot::StyleColorsAuto();
  ImGui::GetIO().IniFilename = NULL;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  if (!LoadWindowsDefaultFont(19.0f))
  {
    MessageBoxW(Window->GetHandle(),
                L"Segoe UI konnte nicht geladen werden. Der ImGui-Standardfont wird verwendet.",
                L"DevTerm", MB_OK | MB_ICONWARNING);
    ImGui::Spectrum::LoadFont(18.0f);
  }
  ImGui_ImplWin32_Init(Window->GetHandle());
  ImGui_ImplOpenGL2_Init();

  int ExitCode = 0;
  {
    CApplication Application;
    bool Running = true;
    while (Running)
    {
      if (!Window->ProcessMessages(ExitCode))
      {
        Running = false;
        continue;
      }

      ImGui_ImplOpenGL2_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();
      Application.Render(Window->GetHandle(), Window->IsActive(),
                         Window->HasBackdrop());
      ImGui::Render();

      int ClientWidth = 0;
      int ClientHeight = 0;
      Window->GetClientSize(ClientWidth, ClientHeight);
      glViewport(0, 0, ClientWidth, ClientHeight);
      const ImVec4& Background = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
      glClearColor(Background.x, Background.y, Background.z,
                   Window->HasBackdrop() ? 0.0f : Background.w);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
      RenderingContext.Present();
      Sleep(10);
    }
  }

  RenderingContext.Destroy();
  Window->Destroy();
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  return ExitCode;
}
