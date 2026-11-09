#pragma once

#include <windows.h>

class CTitleBar
{
public:
  enum { HEIGHT = 38 };

  void Render(HWND Window, bool Active, bool AcrylicEnabled) const;
};
