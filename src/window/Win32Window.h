#pragma once

#include "Window.h"

class CWin32Window : public CWindow
{
public:
  CWin32Window();
  virtual ~CWin32Window();

  virtual bool Create(HINSTANCE Instance, const wchar_t* Title,
                      int Width, int Height);
  virtual void Show(int ShowCommand);
  virtual bool ProcessMessages(int& ExitCode);
  virtual HWND GetHandle() const;
  virtual void GetClientSize(int& Width, int& Height) const;
  virtual bool IsActive() const;
  virtual bool HasBackdrop() const;
  virtual void Destroy();

private:
  static LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message,
                                          WPARAM WParam, LPARAM LParam);
  LRESULT HitTest(POINT Cursor) const;
  bool ApplyVisualEffect();
  void ApplyRoundedCorners();
  void SetShadow(bool Enabled);
  static void AdjustMaximizedClientRect(HWND Window, RECT& Rect);

  HINSTANCE m_Instance;
  HWND m_Handle;
  bool m_ClassRegistered;
  bool m_IsActive;
  bool m_IsMoving;
  bool m_HasBackdrop;
};
