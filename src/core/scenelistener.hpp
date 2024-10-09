#pragma once

namespace conics::core {

class SceneListener {
public:
    virtual void sceneUpdated() = 0;
};

} // namespace conics::core