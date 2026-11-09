#pragma once

#include <cstddef>

class CCommandInput
{
public:
  bool Render(char* Command, std::size_t CommandSize) const;
};
