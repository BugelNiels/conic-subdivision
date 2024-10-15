#pragma once

namespace conis::core {

class Listener {
public:
    virtual void onListenerUpdated() = 0;
};

} // namespace conis::core