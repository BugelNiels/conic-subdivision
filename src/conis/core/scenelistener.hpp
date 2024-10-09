#pragma once

namespace conis::core {

// TODO: rename to listener
class SceneListener {
public:
    virtual void sceneUpdated() = 0;
};

} // namespace conis::core