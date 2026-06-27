#include "compositors/mouse_accel_backend.h"

#include "i18n/i18n.h"
#include "util/string_utils.h"

std::string_view mouseAccelProfileKey(MouseAccelProfile profile) {
  switch (profile) {
  case MouseAccelProfile::Adaptive:
    return "adaptive";
  case MouseAccelProfile::Flat:
    return "flat";
  case MouseAccelProfile::Unknown:
    break;
  }
  return "unknown";
}

std::string mouseAccelProfileLabel(MouseAccelProfile profile) {
  switch (profile) {
  case MouseAccelProfile::Adaptive:
    return i18n::tr("mouse.accel-profiles.adaptive");
  case MouseAccelProfile::Flat:
    return i18n::tr("mouse.accel-profiles.flat");
  case MouseAccelProfile::Unknown:
    break;
  }
  return i18n::tr("mouse.accel-profiles.unknown");
}

std::optional<MouseAccelProfile> parseMouseAccelProfile(std::string_view value) {
  const std::string normalized = StringUtils::toLower(StringUtils::trim(value));
  if (normalized.empty() || normalized == "adaptive" || normalized == "default") {
    return MouseAccelProfile::Adaptive;
  }
  if (normalized == "flat") {
    return MouseAccelProfile::Flat;
  }
  return std::nullopt;
}
