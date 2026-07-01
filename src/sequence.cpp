#if defined(WITH_GUI) || defined(WITH_TUI)

#include <nlohmann/json.hpp>
#include "gen/DFUHelperDeviceInfo.h"
#include "sequence.hpp"

DfuSequence ParseSequence(const std::string& deviceKey)
{
    DfuSequence seq;

    static nlohmann::json j = nlohmann::json::parse(
        DFUHelperDeviceInfo,
        DFUHelperDeviceInfo + DFUHelperDeviceInfo_len
    );

    auto itMap = j["devicemap"].find(deviceKey);
    if (itMap == j["devicemap"].end())
        return seq;

    const std::string deviceType = itMap.value();

    auto itLayout = j["devicelayout"].find(deviceType);
    if (itLayout == j["devicelayout"].end())
        return seq;

    const auto& layout = itLayout.value();

    seq.imageName = layout.value("image_name", "");
    seq.imageWidth = layout.value("image_width", 0);
    seq.imageHeight = layout.value("image_height", 0);
    seq.imageOffsetX = layout.value("image_offset_x", 0);

    if (layout.contains("buttons"))
    {
        for (auto it = layout["buttons"].begin();
             it != layout["buttons"].end();
             ++it)
        {
            DfuButton btn;

            btn.id = it.key();

            const auto& b = it.value();

            btn.name = b.value("name", "");
            btn.x = b.value("x", 0);
            btn.y = b.value("y", 0);

            seq.buttons.push_back(std::move(btn));
        }
    }

    const std::string sequenceKey =
        layout.value("sequence", "");

    auto itSeq = j["sequences"].find(sequenceKey);
    if (itSeq == j["sequences"].end())
        return seq;

    for (const auto& s : itSeq.value())
    {
        DfuStep step;

        step.description = s.value("description", "");
        step.duration = s.value("duration", 0);
        step.action = s.value("action", "");

        if (s.contains("buttons"))
        {
            for (const auto& b : s["buttons"])
                step.activeButtons.push_back(
                    b.get<std::string>()
                );
        }

        seq.steps.push_back(std::move(step));
    }

    return seq;
}

bool SequenceIsSupported(const std::string& deviceKey)
{
    DfuSequence seq = ParseSequence(deviceKey);
    return !seq.steps.empty();
}

#endif // WITH_GUI || WITH_TUI
