#pragma once

#include "compositors/mouse_accel_backend.h"

#include <optional>

namespace compositors::hyprland {
  class HyprlandRuntime;
}

class HyprlandMouseAccelBackend {
public:
  explicit HyprlandMouseAccelBackend(compositors::hyprland::HyprlandRuntime& runtime);

  [[nodiscard]] bool isAvailable() const noexcept;
  [[nodiscard]] std::optional<MouseAccelProfile> currentProfile() const;
  [[nodiscard]] bool setProfile(MouseAccelProfile profile) const;

private:
  [[nodiscard]] std::optional<MouseAccelProfile> queryProfile() const;
  void updateCache(std::optional<MouseAccelProfile> profile) const;

  compositors::hyprland::HyprlandRuntime& m_runtime;
  mutable bool m_cacheSeeded = false;
  mutable std::optional<MouseAccelProfile> m_cachedProfile;
};
