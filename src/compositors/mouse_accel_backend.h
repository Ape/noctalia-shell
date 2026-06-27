#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

enum class MouseAccelProfile : std::uint8_t {
  Unknown = 0,
  Adaptive,
  Flat,
};

[[nodiscard]] std::string_view mouseAccelProfileKey(MouseAccelProfile profile);
[[nodiscard]] std::string mouseAccelProfileLabel(MouseAccelProfile profile);
[[nodiscard]] std::optional<MouseAccelProfile> parseMouseAccelProfile(std::string_view value);

class MouseAccelBackend {
public:
  virtual ~MouseAccelBackend() = default;

  [[nodiscard]] virtual bool isAvailable() const noexcept = 0;
  [[nodiscard]] virtual std::optional<MouseAccelProfile> currentProfile() const = 0;
  [[nodiscard]] virtual bool setProfile(MouseAccelProfile profile) const = 0;

  [[nodiscard]] virtual bool cycleProfile() const {
    const auto current = currentProfile();
    const MouseAccelProfile next =
        current == MouseAccelProfile::Flat ? MouseAccelProfile::Adaptive : MouseAccelProfile::Flat;
    return setProfile(next);
  }
};
