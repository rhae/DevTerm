#include "spectrum.h"

#include "imgui.h"

namespace ImGui
{
namespace Spectrum
{
namespace
{
#define SPECTRUM_ACCENTS \
  Color(0x378EF0), Color(0x2680EB), Color(0x1473E6), Color(0x0D66D0), \
  Color(0xEC5B62), Color(0xE34850), Color(0xD7373F), Color(0xC9252D), \
  Color(0xF29423), Color(0xE68619), Color(0xDA7B11), Color(0xCB6F10), \
  Color(0x33AB84), Color(0x2D9D78), Color(0x268E6C), Color(0x12805C), \
  Color(0x6E6DD0), Color(0x5C5CE0), Color(0x5151D3), Color(0x4646C6), \
  Color(0x58C224), Color(0x51B61B), Color(0x4BA710), Color(0x449906), \
  Color(0xE2499D), Color(0xD83790), Color(0xCE2783), Color(0xBC1C74), \
  Color(0xDFBF00), Color(0xD2B200), Color(0xC4A600), Color(0xB79900), \
  Color(0xCF3EDC), Color(0xC038CC), Color(0xB130BD), Color(0xA228AD), \
  Color(0x1B959A), Color(0x16878C), Color(0x0F797D), Color(0x096C6F), \
  Color(0x85D044), Color(0x7CC33F), Color(0x73B53A), Color(0x6AA834), \
  Color(0x9256D9), Color(0x864CCC), Color(0x7A42BF), Color(0x6F38B1)

const SpectrumPalette LightPalette =
{
  Color(0xFFFFFF), Color(0xFAFAFA), Color(0xF5F5F5), Color(0xEAEAEA),
  Color(0xE1E1E1), Color(0xCACACA), Color(0xB3B3B3), Color(0x8E8E8E),
  Color(0x6E6E6E), Color(0x4B4B4B), Color(0x2C2C2C),
  SPECTRUM_ACCENTS
};

const SpectrumPalette DarkPalette =
{
  Color(0x1D1D1D), Color(0x252525), Color(0x323232), Color(0x3E3E3E),
  Color(0x4D4D4D), Color(0x5C5C5C), Color(0x6B6B6B), Color(0x858585),
  Color(0xB1B1B1), Color(0xD2D2D2), Color(0xFFFFFF),
  SPECTRUM_ACCENTS
};
#undef SPECTRUM_ACCENTS

ImVec4 ToVec4(unsigned int Value)
{
  return ColorConvertU32ToFloat4(Value);
}
}

const SpectrumPalette* Colors = &LightPalette;

void LoadFont(float Size)
{
  ImFontConfig Config;
  Config.SizePixels = Size;
  ImGuiIO& Io = GetIO();
  Io.FontDefault = Io.Fonts->AddFontDefaultVector(&Config);
}

void StyleColorsSpectrum(bool Dark)
{
  Colors = Dark ? &DarkPalette : &LightPalette;
  if (Dark)
    StyleColorsDark();
  else
    StyleColorsLight();

  ImGuiStyle& Style = GetStyle();
  ImVec4* StyleColors = Style.Colors;
  const SpectrumPalette& Palette = *Colors;
  const ImVec4 Accent = ToVec4(Palette.BLUE500);
  const ImVec4 AccentHovered = ToVec4(Palette.BLUE400);
  const ImVec4 AccentActive = ToVec4(Palette.BLUE700);
  const ImVec4 Background = Dark ? ToVec4(Palette.GRAY75) : ToVec4(Color(0xf6f8fa));
  const ImVec4 Surface = Dark ? ToVec4(Palette.GRAY100) : Background;
  const ImVec4 Control = Dark ? ToVec4(Palette.GRAY200) : Background;
  const ImVec4 Hovered = Dark ? ToVec4(Palette.GRAY300) : ToVec4(Color(0xC5C5C5));
  const ImVec4 Active = Dark ? ToVec4(Palette.GRAY400) : ToVec4(Color(0xB5B5B5));
  const ImVec4 Frame = Dark ? Control : ToVec4(Static::WHITE);
  const ImVec4 Border = ToVec4(Palette.GRAY500);

  Style.WindowRounding = 0.0f;
  Style.ChildRounding = 4.0f;
  Style.FrameRounding = 4.0f;
  Style.PopupRounding = 4.0f;
  Style.ScrollbarRounding = 4.0f;
  Style.GrabRounding = 4.0f;
  Style.TabRounding = 4.0f;
  Style.FrameBorderSize = 1.0f;
  Style.WindowBorderSize = 0.0f;
  Style.FramePadding = ImVec2(8.0f, 5.0f);
  Style.ItemSpacing = ImVec2(8.0f, 6.0f);

  StyleColors[ImGuiCol_Text] = ToVec4(Palette.GRAY900);
  StyleColors[ImGuiCol_TextDisabled] = ToVec4(Palette.GRAY600);
  StyleColors[ImGuiCol_WindowBg] = Background;
  StyleColors[ImGuiCol_ChildBg] = Surface;
  StyleColors[ImGuiCol_PopupBg] = Surface;
  StyleColors[ImGuiCol_Border] = Border;
  StyleColors[ImGuiCol_BorderShadow] = ToVec4(Static::NONE);
  StyleColors[ImGuiCol_FrameBg] = Frame;
  StyleColors[ImGuiCol_FrameBgHovered] = Dark ? Hovered : Frame;
  StyleColors[ImGuiCol_FrameBgActive] = Dark ? Active : Frame;
  StyleColors[ImGuiCol_TitleBg] = Surface;
  StyleColors[ImGuiCol_TitleBgActive] = Control;
  StyleColors[ImGuiCol_TitleBgCollapsed] = Surface;
  StyleColors[ImGuiCol_MenuBarBg] = Surface;
  StyleColors[ImGuiCol_ScrollbarBg] = Background;
  StyleColors[ImGuiCol_ScrollbarGrab] = Border;
  StyleColors[ImGuiCol_ScrollbarGrabHovered] = ToVec4(Palette.GRAY600);
  StyleColors[ImGuiCol_ScrollbarGrabActive] = ToVec4(Palette.GRAY700);
  StyleColors[ImGuiCol_CheckMark] = ToVec4(Static::WHITE);
  StyleColors[ImGuiCol_CheckboxSelectedBg] = Accent;
  StyleColors[ImGuiCol_SliderGrab] = Accent;
  StyleColors[ImGuiCol_SliderGrabActive] = AccentHovered;
  StyleColors[ImGuiCol_Button] = Control;
  StyleColors[ImGuiCol_ButtonHovered] = Hovered;
  StyleColors[ImGuiCol_ButtonActive] = Active;
  StyleColors[ImGuiCol_Header] = Control;
  StyleColors[ImGuiCol_HeaderHovered] = Hovered;
  StyleColors[ImGuiCol_HeaderActive] = Active;
  StyleColors[ImGuiCol_Separator] = Border;
  StyleColors[ImGuiCol_SeparatorHovered] = AccentHovered;
  StyleColors[ImGuiCol_SeparatorActive] = AccentActive;
  StyleColors[ImGuiCol_ResizeGrip] = Border;
  StyleColors[ImGuiCol_ResizeGripHovered] = AccentHovered;
  StyleColors[ImGuiCol_ResizeGripActive] = AccentActive;
  StyleColors[ImGuiCol_InputTextCursor] = ToVec4(Palette.GRAY900);
  StyleColors[ImGuiCol_Tab] = Surface;
  StyleColors[ImGuiCol_TabHovered] = Hovered;
  StyleColors[ImGuiCol_TabSelected] = Control;
  StyleColors[ImGuiCol_TabSelectedOverline] = Accent;
  StyleColors[ImGuiCol_TabDimmed] = Surface;
  StyleColors[ImGuiCol_TabDimmedSelected] = Control;
  StyleColors[ImGuiCol_TabDimmedSelectedOverline] = Border;
  StyleColors[ImGuiCol_PlotLines] = Accent;
  StyleColors[ImGuiCol_PlotLinesHovered] = AccentHovered;
  StyleColors[ImGuiCol_PlotHistogram] = Accent;
  StyleColors[ImGuiCol_PlotHistogramHovered] = AccentHovered;
  StyleColors[ImGuiCol_TableHeaderBg] = Control;
  StyleColors[ImGuiCol_TableBorderStrong] = Border;
  StyleColors[ImGuiCol_TableBorderLight] = Hovered;
  StyleColors[ImGuiCol_TableRowBg] = ToVec4(Static::NONE);
  StyleColors[ImGuiCol_TableRowBgAlt] = Surface;
  StyleColors[ImGuiCol_TextLink] = Dark ? AccentHovered : AccentActive;
  StyleColors[ImGuiCol_TextSelectedBg] = ToVec4(color_alpha(110, Palette.BLUE500));
  StyleColors[ImGuiCol_TreeLines] = Border;
  StyleColors[ImGuiCol_DragDropTarget] = Accent;
  StyleColors[ImGuiCol_DragDropTargetBg] = ToVec4(color_alpha(50, Palette.BLUE500));
  StyleColors[ImGuiCol_UnsavedMarker] = Accent;
  StyleColors[ImGuiCol_NavCursor] = Accent;
  StyleColors[ImGuiCol_NavWindowingHighlight] = Accent;
  StyleColors[ImGuiCol_NavWindowingDimBg] = ToVec4(color_alpha(120, Static::BLACK));
  StyleColors[ImGuiCol_ModalWindowDimBg] = ToVec4(color_alpha(120, Static::BLACK));
}
}
}
