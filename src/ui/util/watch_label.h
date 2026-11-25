#pragma once

#include <ucanopen/od/description.h>

#include <string>

namespace ui {

inline std::string watch_label(const ucanopen::od::Item& item) {
    auto label = item.label.ru.empty() ? item.path : item.label.ru;
    if (item.kind == ucanopen::od::Item::Kind::element && !item.label.ru.empty()) {
        label += " " + std::to_string(item.element);
    }

    const auto& unit = item.unit.ru.empty() ? item.unit.en : item.unit.ru;
    if (!unit.empty()) {
        label += " [" + unit + "]";
    }
    return label;
}

} // namespace ui
