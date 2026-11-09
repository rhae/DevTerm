#pragma once

#include <string>
#include <vector>

class CWorkspaceTabs
{
public:
  void Render(const std::vector<std::string>& Log,
              const std::vector<double>& PlotX,
              const std::vector<double>& PlotY,
              bool AutoScroll) const;

private:
  void RenderLog(const std::vector<std::string>& Log,
                 bool AutoScroll) const;
  void RenderPlot(const std::vector<double>& PlotX,
                  const std::vector<double>& PlotY) const;
};
