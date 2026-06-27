#include "shell/bar/widgets/mouse_accel_profile_widget.h"

#include "compositors/compositor_platform.h"
#include "core/log.h"
#include "render/core/renderer.h"
#include "render/scene/input_area.h"
#include "ui/builders.h"
#include "ui/palette.h"
#include "ui/style.h"

#include <algorithm>
#include <cmath>
#include <linux/input-event-codes.h>
#include <memory>
#include <utility>

namespace {

  constexpr Logger kLog("mouse_accel_profile_widget");
  constexpr std::string_view kUnknownLabel = "--";

} // namespace

MouseAccelProfileWidget::MouseAccelProfileWidget(
    CompositorPlatform& platform, bool showIcon, bool showLabel, std::string glyph
)
    : m_platform(platform), m_showIcon(showIcon), m_showLabel(showLabel), m_glyphName(std::move(glyph)) {}

void MouseAccelProfileWidget::create() {
  auto area = std::make_unique<InputArea>();
  area->setOnLeave([this]() { m_clickArmed = false; });
  area->setOnPress([this](const InputArea::PointerData& data) {
    if (!data.pressed) {
      return;
    }
    m_clickArmed = data.button == BTN_LEFT;
  });
  area->setOnClick([this](const InputArea::PointerData& data) {
    if (!m_clickArmed || data.button != BTN_LEFT) {
      return;
    }
    m_clickArmed = false;
    toggleProfile();
  });

  area->addChild(
      ui::glyph({
          .out = &m_glyph,
          .glyph = m_glyphName,
          .glyphSize = Style::baseGlyphSize * m_contentScale,
          .color = widgetIconColorOr(colorSpecFromRole(ColorRole::OnSurface)),
      })
  );

  area->addChild(
      ui::label({
          .out = &m_label,
          .text = std::string(kUnknownLabel),
          .fontSize = Style::fontSizeBody * m_contentScale,
          .fontFamily = labelFontFamily(),
          .fontWeight = labelFontWeight(),
      })
  );

  setRoot(std::move(area));
}

void MouseAccelProfileWidget::doLayout(Renderer& renderer, float containerWidth, float containerHeight) {
  if (m_label == nullptr || root() == nullptr) {
    return;
  }

  m_isVertical = containerHeight > containerWidth;
  sync(renderer);

  const bool showIcon = m_showIcon && m_glyph != nullptr;
  if (m_glyph != nullptr) {
    m_glyph->setVisible(m_showIcon);
  }
  if (showIcon) {
    m_glyph->setGlyphSize(Style::baseGlyphSize * m_contentScale);
    m_glyph->setColor(widgetIconColorOr(colorSpecFromRole(ColorRole::OnSurface)));
    m_glyph->measure(renderer);
    if (m_glyph->width() <= 0.0f && m_glyphName == "mouse") {
      m_glyph->setGlyph("pointer");
      m_glyph->measure(renderer);
    }
  }

  m_label->setColor(widgetForegroundOr(colorSpecFromRole(ColorRole::OnSurface)));
  m_label->setVisible(m_showLabel);
  m_label->setTextAlign(m_isVertical ? TextAlign::Center : TextAlign::Start);
  if (m_showLabel) {
    m_label->setMinWidth(0.0f);
    m_label->measure(renderer);
  }

  if (m_isVertical) {
    const float iconW = showIcon ? m_glyph->width() : 0.0f;
    const float iconH = showIcon ? m_glyph->height() : 0.0f;
    const float labelW = m_showLabel ? m_label->width() : 0.0f;
    const float labelH = m_showLabel ? m_label->height() : 0.0f;
    const float w = std::max(iconW, labelW);
    float y = 0.0f;
    if (showIcon) {
      m_glyph->setPosition(std::round((w - iconW) * 0.5f), y);
      y += iconH;
    }
    if (m_showLabel) {
      m_label->setPosition(std::round((w - labelW) * 0.5f), y);
      y += labelH;
    }
    root()->setSize(w, y);
  } else {
    const float spacing = Style::spaceXs;
    float x = 0.0f;
    const float iconH = showIcon ? m_glyph->height() : 0.0f;
    const float labelH = m_showLabel ? m_label->height() : 0.0f;
    const float h = std::max(iconH, labelH);
    if (showIcon) {
      const float glyphY = std::round((h - m_glyph->height()) * 0.5f);
      m_glyph->setPosition(0.0f, glyphY);
      x += m_glyph->width();
      if (m_showLabel) {
        x += spacing;
      }
    }
    if (m_showLabel) {
      const float labelY = std::round((h - m_label->height()) * 0.5f);
      m_label->setPosition(x, labelY);
      root()->setSize(m_label->x() + m_label->width(), h);
    } else {
      root()->setSize(x, h);
    }
  }
}

void MouseAccelProfileWidget::doUpdate(Renderer& renderer) { sync(renderer); }

void MouseAccelProfileWidget::sync(Renderer& renderer) {
  if (m_label == nullptr) {
    return;
  }

  const bool available = m_platform.hasMouseAccelBackend();
  const auto profile = available ? m_platform.currentMouseAccelProfile() : std::optional<MouseAccelProfile>{};

  const std::string label = profile.has_value() ? formatProfileLabel(*profile) : std::string(kUnknownLabel);

  if (profile == m_lastProfile
      && label == m_lastLabel
      && available == m_lastAvailable
      && m_isVertical == m_lastVertical) {
    return;
  }

  m_lastProfile = profile;
  m_lastLabel = label;
  m_lastAvailable = available;
  m_lastVertical = m_isVertical;

  if (m_glyph != nullptr) {
    m_glyph->setVisible(m_showIcon);
    m_glyph->setColor(
        available ? widgetIconColorOr(colorSpecFromRole(ColorRole::OnSurface))
                  : colorSpecFromRole(ColorRole::OnSurfaceVariant)
    );
  }
  m_label->setVisible(m_showLabel);
  if (m_showLabel) {
    m_label->setFontSize((m_isVertical ? Style::fontSizeCaption : Style::fontSizeBody) * m_contentScale);
    m_label->setText(label);
    m_label->setColor(
        available ? widgetForegroundOr(colorSpecFromRole(ColorRole::OnSurface))
                  : colorSpecFromRole(ColorRole::OnSurfaceVariant)
    );
    m_label->measure(renderer);
  }

  if (auto* area = static_cast<InputArea*>(root()); area != nullptr) {
    area->setEnabled(available);
  }

  if (auto* node = root(); node != nullptr) {
    node->setOpacity(available ? 1.0f : 0.55f);
  }

  requestRedraw();
}

void MouseAccelProfileWidget::toggleProfile() {
  if (!m_platform.hasMouseAccelBackend()) {
    return;
  }

  const auto current = m_platform.currentMouseAccelProfile();
  const MouseAccelProfile next =
      current == MouseAccelProfile::Flat ? MouseAccelProfile::Adaptive : MouseAccelProfile::Flat;
  if (!m_platform.setMouseAccelProfile(next)) {
    kLog.warn("mouse_accel_profile: compositor backend failed to set profile");
    return;
  }

  requestUpdate();
}

std::string MouseAccelProfileWidget::formatProfileLabel(MouseAccelProfile profile) {
  if (profile == MouseAccelProfile::Unknown) {
    return std::string(kUnknownLabel);
  }
  return mouseAccelProfileLabel(profile);
}
