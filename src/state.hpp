#if defined(WITH_GUI) || defined(WITH_TUI)

#ifndef STATE_H
#define STATE_H

#include <string>
#include <cstdint>

enum class DeviceMode
{
    None,
    Normal,
    Recovery,
    DFU
};

struct DeviceState
{
    bool connected = false;
    bool isSupported = false;
    DeviceMode mode = DeviceMode::None;
    std::string productVersion;
    std::string productType;
    std::string displayName;
    std::string udid;
    uint64_t ecid;
};

#endif // STATE_H

#endif // WITH_GUI || WITH_TUI
