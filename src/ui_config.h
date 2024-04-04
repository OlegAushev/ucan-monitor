#pragma once


#include <imgui.h>


namespace ui {
namespace colors {


inline const ImVec4 green_toggle_button         = (ImVec4)ImColor::HSV(0.35f, 0.6f, 0.6f);
inline const ImVec4 green_toggle_button_hovered = (ImVec4)ImColor::HSV(0.35f, 0.8f, 0.8f);
inline const ImVec4 green_toggle_button_active  = (ImVec4)ImColor::HSV(0.35f, 0.7f, 0.7f);

inline const ImVec4 icon_inactive               = (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.25f);

// GNOME Palette
inline constexpr auto blue_1    = IM_COL32(153, 193, 241, 255);
inline constexpr auto blue_2    = IM_COL32(98, 160, 234, 255);
inline constexpr auto blue_3    = IM_COL32(53, 132, 228, 255);
inline constexpr auto blue_4    = IM_COL32(28, 113, 216, 255);
inline constexpr auto blue_5    = IM_COL32(26, 95, 180, 255);

inline constexpr auto green_1   = IM_COL32(143, 240, 164, 255);
inline constexpr auto green_2   = IM_COL32(87, 227, 137, 255);
inline constexpr auto green_3   = IM_COL32(51, 209, 122, 255);
inline constexpr auto green_4   = IM_COL32(46, 194, 126, 255);
inline constexpr auto green_5   = IM_COL32(38, 162, 105, 255);

inline constexpr auto yellow_1  = IM_COL32(249, 240, 107, 255);
inline constexpr auto yellow_2  = IM_COL32(248, 228, 92, 255);
inline constexpr auto yellow_3  = IM_COL32(246, 211, 45, 255);
inline constexpr auto yellow_4  = IM_COL32(245, 194, 17, 255);
inline constexpr auto yellow_5  = IM_COL32(229, 165, 10, 255);

inline constexpr auto orange_1  = IM_COL32(255, 190, 111, 255);
inline constexpr auto orange_2  = IM_COL32(255, 163, 72, 255);
inline constexpr auto orange_3  = IM_COL32(255, 120, 0, 255);
inline constexpr auto orange_4  = IM_COL32(230, 97, 0, 255);
inline constexpr auto orange_5  = IM_COL32(198, 70, 0, 255);

inline constexpr auto red_1     = IM_COL32(246, 97, 81, 255);
inline constexpr auto red_2     = IM_COL32(237, 51, 59, 255);
inline constexpr auto red_3     = IM_COL32(224, 27, 36, 255);
inline constexpr auto red_4     = IM_COL32(192, 28, 40, 255);
inline constexpr auto red_5     = IM_COL32(165, 29, 45, 255);

inline constexpr auto purple_1  = IM_COL32(220, 138, 221, 255);
inline constexpr auto purple_2  = IM_COL32(192, 97, 203, 255);
inline constexpr auto purple_3  = IM_COL32(145, 65, 172, 255);
inline constexpr auto purple_4  = IM_COL32(129, 61, 156, 255);
inline constexpr auto purple_5  = IM_COL32(97, 53, 131, 255);

inline constexpr auto brown_1   = IM_COL32(205, 171, 143, 255);
inline constexpr auto brown_2   = IM_COL32(181, 131, 90, 255);
inline constexpr auto brown_3   = IM_COL32(152, 106, 6, 255);
inline constexpr auto brown_4   = IM_COL32(134, 94, 60, 255);
inline constexpr auto brown_5   = IM_COL32(99, 69, 44, 255);

inline constexpr auto light_1   = IM_COL32(255, 255, 255, 255);
inline constexpr auto light_2   = IM_COL32(246, 245, 244, 255);
inline constexpr auto light_3   = IM_COL32(222, 221, 218, 255);
inline constexpr auto light_4   = IM_COL32(192, 191, 188, 255);
inline constexpr auto light_5   = IM_COL32(154, 153, 150, 255);

inline constexpr auto dark_1    = IM_COL32(119, 118, 123, 255);
inline constexpr auto dark_2    = IM_COL32(94, 92, 100, 255);
inline constexpr auto dark_3    = IM_COL32(61, 56, 70, 255);
inline constexpr auto dark_4    = IM_COL32(36, 31, 49, 255);
inline constexpr auto dark_5    = IM_COL32(0, 0, 0, 255);


} // namespace colors
} // namespace ui
