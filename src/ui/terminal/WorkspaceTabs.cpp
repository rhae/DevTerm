#include "WorkspaceTabs.h"

#include "imgui.h"
#include "implot.h"

void CWorkspaceTabs::Render(const std::vector<std::string>& Log,
                            const std::vector<double>& PlotX,
                            const std::vector<double>& PlotY,
                            bool AutoScroll) const
{
  if (!ImGui::BeginTabBar("Arbeitsbereich"))
    return;

  if (ImGui::BeginTabItem("Protokoll"))
  {
    RenderLog(Log, AutoScroll);
    ImGui::EndTabItem();
  }
  if (ImGui::BeginTabItem("Plot"))
  {
    RenderPlot(PlotX, PlotY);
    ImGui::EndTabItem();
  }
  ImGui::EndTabBar();
}

void CWorkspaceTabs::RenderLog(const std::vector<std::string>& Log,
                               bool AutoScroll) const
{
  ImGui::BeginChild("Protokollinhalt",
                    ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
  for (size_t Index = 0; Index < Log.size(); ++Index)
    ImGui::TextUnformatted(Log[Index].c_str());
  if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    ImGui::SetScrollHereY(1.0f);
  ImGui::EndChild();
}

void CWorkspaceTabs::RenderPlot(const std::vector<double>& PlotX,
                                const std::vector<double>& PlotY) const
{
  if (PlotY.empty())
  {
    ImGui::TextDisabled(
        "Numerische Einzelwerte aus Geraeteantworten erscheinen hier.");
  }
  else if (ImPlot::BeginPlot("Messwerte", ImVec2(-1, -1)))
  {
    ImPlot::SetupAxes("Messung", "Wert");
    ImPlot::PlotLine("Antwort", &PlotX[0], &PlotY[0],
                     static_cast<int>(PlotY.size()));
    ImPlot::EndPlot();
  }
}
