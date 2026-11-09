#include "CWindowsFont.h"

#include "imgui.h"

#include <windows.h>
#include <vector>

bool LoadWindowsFont(const std::wstring& Path, float Size)
{
  int Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, Path.c_str(), -1,
                                   NULL, 0, NULL, NULL);
  if (Length <= 0)
    return false;
  std::vector<char> Utf8Path(static_cast<size_t>(Length));
  if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, Path.c_str(), -1,
                          &Utf8Path[0], Length, NULL, NULL) == 0)
    return false;

  ImFontConfig Config;
  Config.Flags |= ImFontFlags_NoLoadError;
  ImFont* Font = ImGui::GetIO().Fonts->AddFontFromFileTTF(&Utf8Path[0], Size, &Config);
  if (Font == NULL)
    return false;
  ImGui::GetIO().FontDefault = Font;
  return true;
}

bool LoadWindowsDefaultFont(float Size)
{
  wchar_t Directory[MAX_PATH];
  UINT Length = GetWindowsDirectoryW(Directory, MAX_PATH);
  if (Length == 0 || Length >= MAX_PATH)
    return false;
  return LoadWindowsFont(std::wstring(Directory, Length) + L"\\Fonts\\segoeui.ttf", Size);
}
