#pragma once

#include <string>

class CStatusBar
{
public:
  void Render(bool Connected, const std::string& Status) const;
};
