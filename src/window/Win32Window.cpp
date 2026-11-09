#include "Win32Window.h"

#include "../ui/shell/TitleBar.h"
#include "imgui_impl_win32.h"
#include <dwmapi.h>
#include <windowsx.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

namespace
{
const wchar_t* WindowClassName = L"DevTermWindow";
const int TitleBarHeight = CTitleBar::HEIGHT;
const int CornerRadius = 12;

enum WindowCompositionAttribute
{
  WindowCompositionAccentPolicy = 19
};

enum AccentState
{
  AccentEnableGradient = 1,
  AccentEnableAcrylicBlurBehind = 4
};

struct AccentPolicy
{
  int State;
  DWORD Flags;
  DWORD GradientColor;
  DWORD AnimationId;
};

struct WindowCompositionAttributeData
{
  WindowCompositionAttribute Attribute;
  PVOID Data;
  SIZE_T DataSize;
};

typedef BOOL(WINAPI* SetWindowCompositionAttributeFunction)(
    HWND, WindowCompositionAttributeData*);

bool IsCompositionEnabled()
{
  BOOL Enabled = FALSE;
  return SUCCEEDED(DwmIsCompositionEnabled(&Enabled)) && Enabled != FALSE;
}
}

CWin32Window::CWin32Window()
  : m_Instance(NULL),
    m_Handle(NULL),
    m_ClassRegistered(false),
    m_IsActive(false),
    m_IsMoving(false),
    m_HasBackdrop(false)
{
}

CWin32Window::~CWin32Window()
{
  Destroy();
}

bool CWin32Window::Create(HINSTANCE Instance, const wchar_t* Title,
                          int Width, int Height)
{
  m_Instance = Instance;

  WNDCLASSEXW WindowClass = {};
  WindowClass.cbSize = sizeof(WindowClass);
  WindowClass.style = CS_CLASSDC;
  WindowClass.lpfnWndProc = WindowProcedure;
  WindowClass.hInstance = m_Instance;
  WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  WindowClass.lpszClassName = WindowClassName;

  if (!RegisterClassExW(&WindowClass))
    return false;
  m_ClassRegistered = true;

  const DWORD Style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX |
                      WS_MAXIMIZEBOX | WS_SYSMENU;
  m_Handle = CreateWindowExW(0, WindowClassName, Title, Style,
                           100, 100, Width, Height, NULL, NULL,
                           m_Instance, this);
  if (m_Handle == NULL)
  {
    Destroy();
    return false;
  }
  SetShadow(true);
  m_HasBackdrop = ApplyVisualEffect();
  return true;
}

void CWin32Window::Show(int ShowCommand)
{
  ShowWindow(m_Handle, ShowCommand);
  UpdateWindow(m_Handle);
}

bool CWin32Window::ProcessMessages(int& ExitCode)
{
  MSG Message;
  while (PeekMessageW(&Message, NULL, 0, 0, PM_REMOVE))
  {
    if (Message.message == WM_QUIT)
    {
      ExitCode = static_cast<int>(Message.wParam);
      return false;
    }
    TranslateMessage(&Message);
    DispatchMessageW(&Message);
  }
  return true;
}

HWND CWin32Window::GetHandle() const
{
  return m_Handle;
}

void CWin32Window::GetClientSize(int& Width, int& Height) const
{
  RECT ClientArea = {};
  if (m_Handle != NULL && GetClientRect(m_Handle, &ClientArea))
  {
    Width = ClientArea.right - ClientArea.left;
    Height = ClientArea.bottom - ClientArea.top;
  }
  else
  {
    Width = 0;
    Height = 0;
  }
}

bool CWin32Window::HasBackdrop() const
{
  return m_HasBackdrop;
}

bool CWin32Window::IsActive() const
{
  return m_IsActive;
}

void CWin32Window::Destroy()
{
  if (m_Handle != NULL)
  {
    DestroyWindow(m_Handle);
    m_Handle = NULL;
  }
  if (m_ClassRegistered)
  {
    UnregisterClassW(WindowClassName, m_Instance);
    m_ClassRegistered = false;
  }
}

LRESULT CWin32Window::HitTest(POINT Cursor) const
{
  RECT WindowRect;
  if (!GetWindowRect(m_Handle, &WindowRect))
    return HTNOWHERE;

  const bool Maximized = IsZoomed(m_Handle) != FALSE;
  if (!Maximized)
  {
    const int BorderX = GetSystemMetrics(SM_CXFRAME) +
                        GetSystemMetrics(SM_CXPADDEDBORDER);
    const int BorderY = GetSystemMetrics(SM_CYFRAME) +
                        GetSystemMetrics(SM_CXPADDEDBORDER);
    const bool Left = Cursor.x < WindowRect.left + BorderX;
    const bool Right = Cursor.x >= WindowRect.right - BorderX;
    const bool Top = Cursor.y < WindowRect.top + BorderY;
    const bool Bottom = Cursor.y >= WindowRect.bottom - BorderY;

    if (Top && Left)
      return HTTOPLEFT;
    if (Top && Right)
      return HTTOPRIGHT;
    if (Bottom && Left)
      return HTBOTTOMLEFT;
    if (Bottom && Right)
      return HTBOTTOMRIGHT;
    if (Left)
      return HTLEFT;
    if (Right)
      return HTRIGHT;
    if (Top)
      return HTTOP;
    if (Bottom)
      return HTBOTTOM;
  }

  if (Cursor.y < WindowRect.top + TitleBarHeight)
  {
    if (Cursor.x < WindowRect.right - 3 * 44 - 6)
      return HTCAPTION;
    return HTCLIENT;
  }
  return HTCLIENT;
}

bool CWin32Window::ApplyVisualEffect()
{
  if (m_Handle == NULL)
    return false;

  HMODULE User32 = GetModuleHandleW(L"user32.dll");
  SetWindowCompositionAttributeFunction SetCompositionAttribute = NULL;
  if (User32 != NULL)
  {
    SetCompositionAttribute =
        reinterpret_cast<SetWindowCompositionAttributeFunction>(
            GetProcAddress(User32, "SetWindowCompositionAttribute"));
  }

  bool BackdropApplied = false;
  if (SetCompositionAttribute != NULL && IsCompositionEnabled())
  {
    AccentPolicy Policy = {};
    Policy.State = AccentEnableAcrylicBlurBehind;
    Policy.GradientColor = 0xB3000000;

    WindowCompositionAttributeData Data = {};
    Data.Attribute = WindowCompositionAccentPolicy;
    Data.Data = &Policy;
    Data.DataSize = sizeof(Policy);
    BackdropApplied = SetCompositionAttribute(m_Handle, &Data) != FALSE;
  }

  if (!BackdropApplied && SetCompositionAttribute != NULL)
  {
    DWORD Color = 0xB3000000;
    DWORD ColorizationColor = 0;
    BOOL OpaqueBlend = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&ColorizationColor, &OpaqueBlend)))
    {
      const DWORD Red = (ColorizationColor >> 16) & 0xFF;
      const DWORD Green = (ColorizationColor >> 8) & 0xFF;
      const DWORD Blue = ColorizationColor & 0xFF;
      Color = (0xB3 << 24) | (Blue << 16) | (Green << 8) | Red;
    }

    AccentPolicy Policy = {};
    Policy.State = AccentEnableGradient;
    Policy.GradientColor = Color;
    WindowCompositionAttributeData Data = {};
    Data.Attribute = WindowCompositionAccentPolicy;
    Data.Data = &Policy;
    Data.DataSize = sizeof(Policy);
    BackdropApplied = SetCompositionAttribute(m_Handle, &Data) != FALSE;
  }

  ApplyRoundedCorners();
  return BackdropApplied;
}

void CWin32Window::ApplyRoundedCorners()
{
  if (m_Handle == NULL)
    return;

  const DWORD CornerPreferenceAttribute = 33;
  const int RoundedCornerPreference = 2;
  HRESULT Result = DwmSetWindowAttribute(
      m_Handle, CornerPreferenceAttribute, &RoundedCornerPreference,
      sizeof(RoundedCornerPreference));
  if (SUCCEEDED(Result))
  {
    SetWindowRgn(m_Handle, NULL, TRUE);
    return;
  }

  if (IsZoomed(m_Handle))
  {
    SetWindowRgn(m_Handle, NULL, TRUE);
    return;
  }

  RECT WindowRect;
  if (!GetWindowRect(m_Handle, &WindowRect))
    return;

  const int Width = WindowRect.right - WindowRect.left;
  const int Height = WindowRect.bottom - WindowRect.top;
  HRGN Region = CreateRoundRectRgn(0, 0, Width + 1, Height + 1,
                                   CornerRadius * 2, CornerRadius * 2);
  if (Region != NULL && !SetWindowRgn(m_Handle, Region, TRUE))
    DeleteObject(Region);
}

void CWin32Window::SetShadow(bool Enabled)
{
  if (!IsCompositionEnabled() || m_Handle == NULL)
    return;

  const MARGINS Margins = { Enabled ? 1 : 0, Enabled ? 1 : 0,
                            Enabled ? 1 : 0, Enabled ? 1 : 0 };
  DwmExtendFrameIntoClientArea(m_Handle, &Margins);
}

void CWin32Window::AdjustMaximizedClientRect(HWND Window, RECT& Rect)
{
  if (!IsZoomed(Window))
    return;

  HMONITOR Monitor = MonitorFromWindow(Window, MONITOR_DEFAULTTONEAREST);
  if (Monitor == NULL)
    return;

  MONITORINFO MonitorInfo = {};
  MonitorInfo.cbSize = sizeof(MonitorInfo);
  if (GetMonitorInfoW(Monitor, &MonitorInfo))
    Rect = MonitorInfo.rcWork;
}

LRESULT CALLBACK CWin32Window::WindowProcedure(HWND Window, UINT Message,
                                               WPARAM WParam, LPARAM LParam)
{
  if (ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
    return 1;

  CWin32Window* Instance = reinterpret_cast<CWin32Window*>(
      GetWindowLongPtrW(Window, GWLP_USERDATA));
  if (Message == WM_NCCREATE)
  {
    CREATESTRUCTW* CreateInfo = reinterpret_cast<CREATESTRUCTW*>(LParam);
    Instance = static_cast<CWin32Window*>(CreateInfo->lpCreateParams);
    if (Instance == NULL)
      return FALSE;
    SetWindowLongPtrW(Window, GWLP_USERDATA,
                      reinterpret_cast<LONG_PTR>(Instance));
    Instance->m_Handle = Window;
  }

  if (Instance != NULL)
  {
    switch (Message)
    {
    case WM_NCCALCSIZE:
      if (Instance->m_Handle != NULL)
      {
        if (WParam != FALSE)
        {
          NCCALCSIZE_PARAMS* Parameters =
              reinterpret_cast<NCCALCSIZE_PARAMS*>(LParam);
          AdjustMaximizedClientRect(Window, Parameters->rgrc[0]);
        }
        return 0;
      }
      break;
    case WM_NCHITTEST:
    {
      LRESULT Hit = Instance->HitTest(
          POINT{GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam)});
      if (Hit != HTNOWHERE)
        return Hit;
      break;
    }
    case WM_NCACTIVATE:
      Instance->m_IsActive = WParam != FALSE;
      break;
    case WM_ACTIVATE:
      Instance->m_IsActive = LOWORD(WParam) != WA_INACTIVE;
      break;
    case WM_ENTERSIZEMOVE:
      Instance->m_IsMoving = true;
      break;
    case WM_EXITSIZEMOVE:
      Instance->m_IsMoving = false;
      InvalidateRect(Window, NULL, FALSE);
      UpdateWindow(Window);
      break;
    case WM_SIZE:
      if (WParam != SIZE_MINIMIZED)
        Instance->ApplyRoundedCorners();
      break;
    case WM_DPICHANGED:
    {
      RECT* SuggestedRect = reinterpret_cast<RECT*>(LParam);
      SetWindowPos(Window, NULL, SuggestedRect->left, SuggestedRect->top,
                   SuggestedRect->right - SuggestedRect->left,
                   SuggestedRect->bottom - SuggestedRect->top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
      Instance->m_HasBackdrop = Instance->ApplyVisualEffect();
      return 0;
    }
    case WM_DWMCOMPOSITIONCHANGED:
    case WM_DWMCOLORIZATIONCOLORCHANGED:
      Instance->m_HasBackdrop = Instance->ApplyVisualEffect();
      break;
    case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      BeginPaint(Window, &Paint);
      EndPaint(Window, &Paint);
      return 0;
    }
    case WM_NCDESTROY:
      SetWindowLongPtrW(Window, GWLP_USERDATA, 0);
      Instance->m_Handle = NULL;
      break;
    default:
      break;
    }
  }

  if (Message == WM_SYSCOMMAND && (WParam & 0xfff0) == SC_KEYMENU)
    return 0;
  if (Message == WM_CLOSE)
  {
    DestroyWindow(Window);
    return 0;
  }
  if (Message == WM_DESTROY)
  {
    if (Instance != NULL)
      Instance->m_Handle = NULL;
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(Window, Message, WParam, LParam);
}
