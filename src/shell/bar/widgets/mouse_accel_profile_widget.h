#pragma once

#include "compositors/mouse_accel_backend.h"
#include "shell/bar/widget.h"

#include <optional>
#include <string>

class CompositorPlatform;
class Glyph;
class Label;
class Renderer;

class MouseAccelProfileWidget : public Widget {
public:
  MouseAccelProfileWidget(CompositorPlatform& platform, bool showIcon, bool showLabel, std::string glyph = "mouse");

  static std::string formatProfileLabel(MouseAccelProfile profile);

  void create() override;

private:
  void doLayout(Renderer& renderer, float containerWidth, float containerHeight) override;
  void doUpdate(Renderer& renderer) override;
  void sync(Renderer& renderer);
  void toggleProfile();

  CompositorPlatform& m_platform;
  bool m_showIcon = true;
  bool m_showLabel = true;
  std::string m_glyphName = "mouse";

  Glyph* m_glyph = nullptr;
  Label* m_label = nullptr;

  std::optional<MouseAccelProfile> m_lastProfile;
  std::string m_lastLabel;
  bool m_lastAvailable = false;
  bool m_lastVertical = false;
  bool m_clickArmed = false;
  bool m_isVertical = false;
};
