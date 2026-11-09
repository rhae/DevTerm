#pragma once

#include <windows.h>

class CWindow
{
public:
  virtual ~CWindow();

  virtual bool Create(HINSTANCE Instance, const wchar_t* Title,
                      int Width, int Height) = 0;
  virtual void Show(int ShowCommand) = 0;
  virtual bool ProcessMessages(int& ExitCode) = 0;
  virtual HWND GetHandle() const = 0;
  virtual void GetClientSize(int& Width, int& Height) const = 0;
  virtual bool IsActive() const = 0;
  virtual bool HasBackdrop() const = 0;
  virtual void Destroy() = 0;
};
