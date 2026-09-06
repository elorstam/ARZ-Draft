#include "cad/layers/Layer.h"

#include <utility>

namespace arz::cad {

Layer::Layer(
    LayerId id,
    std::string name
)
    : id_(id),
      name_(std::move(name)) {
}

LayerId Layer::id() const noexcept {
    return id_;
}

const std::string& Layer::name() const noexcept {
    return name_;
}

const Color& Layer::color() const noexcept {
    return color_;
}

LineTypeId Layer::lineTypeId() const noexcept {
    return lineTypeId_;
}

LineWeight Layer::lineWeight() const noexcept {
    return lineWeight_;
}

bool Layer::visible() const noexcept {
    return visible_;
}

bool Layer::frozen() const noexcept {
    return frozen_;
}

bool Layer::locked() const noexcept {
    return locked_;
}

bool Layer::plottable() const noexcept {
    return plottable_;
}

void Layer::setName(std::string name) {
    name_ = std::move(name);
}

void Layer::setColor(Color color) noexcept {
    color_ = color;
}

void Layer::setLineTypeId(LineTypeId id) noexcept {
    lineTypeId_ = id;
}

void Layer::setLineWeight(LineWeight value) noexcept {
    lineWeight_ = value;
}

void Layer::setVisible(bool value) noexcept {
    visible_ = value;
}

void Layer::setFrozen(bool value) noexcept {
    frozen_ = value;
}

void Layer::setLocked(bool value) noexcept {
    locked_ = value;
}

void Layer::setPlottable(bool value) noexcept {
    plottable_ = value;
}

}
