#include "compositors/hyprland/hyprland_mouse_accel_backend.h"

#include "compositors/hyprland/hyprland_runtime.h"
#include "core/log.h"
#include "util/string_utils.h"

#include <format>
#include <json.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace {

  constexpr Logger kLog("hyprland_mouse_accel");

  [[nodiscard]] std::optional<MouseAccelProfile> parseJsonProfile(const nlohmann::json& json) {
    if (json.is_string()) {
      return parseMouseAccelProfile(json.get<std::string>());
    }
    if (!json.is_object()) {
      return std::nullopt;
    }

    for (const char* key : {"str", "value", "data"}) {
      const auto it = json.find(key);
      if (it != json.end() && it->is_string()) {
        if (auto profile = parseMouseAccelProfile(it->get<std::string>()); profile.has_value()) {
          return profile;
        }
      }
    }

    return std::nullopt;
  }

  [[nodiscard]] std::optional<MouseAccelProfile> parseTextProfile(std::string_view response) {
    const std::string trimmed = StringUtils::trim(response);
    if (auto profile = parseMouseAccelProfile(trimmed); profile.has_value()) {
      return profile;
    }

    std::size_t offset = 0;
    while (offset < response.size()) {
      const std::size_t end = response.find('\n', offset);
      const std::string_view line =
          response.substr(offset, end == std::string_view::npos ? std::string_view::npos : end - offset);
      const std::size_t colon = line.find(':');
      if (colon != std::string_view::npos) {
        const std::string key = StringUtils::toLower(StringUtils::trim(line.substr(0, colon)));
        if (key == "str" || key == "value" || key == "data") {
          if (auto profile = parseMouseAccelProfile(line.substr(colon + 1)); profile.has_value()) {
            return profile;
          }
        }
      }
      if (end == std::string_view::npos) {
        break;
      }
      offset = end + 1;
    }

    return std::nullopt;
  }

  [[nodiscard]] std::string responseSummary(const std::optional<std::string>& response) {
    if (!response.has_value()) {
      return "<no response>";
    }
    std::string summary = StringUtils::trim(*response);
    if (summary.empty()) {
      return "<empty response>";
    }
    constexpr std::size_t kMaxSummaryLength = 220;
    if (summary.size() > kMaxSummaryLength) {
      summary.resize(kMaxSummaryLength);
      summary += "...";
    }
    return summary;
  }

  [[nodiscard]] bool responseAccepted(std::string_view command, const std::optional<std::string>& response) {
    if (!response.has_value()) {
      kLog.warn("mouse accel command '{}' failed: {}", command, responseSummary(response));
      return false;
    }
    const std::string lower = StringUtils::toLower(StringUtils::trim(*response));
    const bool accepted = !lower.starts_with("error") && !lower.starts_with("unknown");
    if (!accepted) {
      kLog.warn("mouse accel command '{}' failed: {}", command, responseSummary(response));
    }
    return accepted;
  }

} // namespace

HyprlandMouseAccelBackend::HyprlandMouseAccelBackend(compositors::hyprland::HyprlandRuntime& runtime)
    : m_runtime(runtime) {}

bool HyprlandMouseAccelBackend::isAvailable() const noexcept { return !m_runtime.requestSocketPath().empty(); }

std::optional<MouseAccelProfile> HyprlandMouseAccelBackend::currentProfile() const {
  if (!isAvailable()) {
    return std::nullopt;
  }

  if (!m_cacheSeeded) {
    updateCache(queryProfile());
  }
  return m_cachedProfile;
}

std::optional<MouseAccelProfile> HyprlandMouseAccelBackend::queryProfile() const {
  constexpr std::string_view option = "input.accel_profile";
  if (const auto json = m_runtime.requestJson(std::format("j/getoption {}", option)); json.has_value()) {
    if (auto profile = parseJsonProfile(*json); profile.has_value()) {
      return profile;
    }
  }

  if (const auto response = m_runtime.request(std::format("getoption {}", option)); response.has_value()) {
    return parseTextProfile(*response);
  }

  return std::nullopt;
}

void HyprlandMouseAccelBackend::updateCache(std::optional<MouseAccelProfile> profile) const {
  m_cacheSeeded = true;
  m_cachedProfile = profile;
}

bool HyprlandMouseAccelBackend::setProfile(MouseAccelProfile profile) const {
  if (!isAvailable()) {
    return false;
  }
  const std::string_view value = mouseAccelProfileKey(profile);
  if (value != "adaptive" && value != "flat") {
    return false;
  }

  const std::string command = std::format("eval hl.config({{ input = {{ accel_profile = \"{}\" }} }})", value);
  if (!responseAccepted(command, m_runtime.request(command))) {
    return false;
  }

  updateCache(profile);
  return true;
}
