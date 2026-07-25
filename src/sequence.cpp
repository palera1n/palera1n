/*
 * palera1n - https://palera.in
 *
 * Copyright (C) 2026 palera1n team
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#if defined(WITH_GUI) || defined(WITH_TUI)

#include <nlohmann/json.hpp>
#include "gen/embedded/DFUHelperDeviceInfo.h"
#include "sequence.hpp"

DfuSequence ParseSequence(const std::string& deviceKey)
{
    DfuSequence seq;

    static nlohmann::json j = nlohmann::json::parse(
        embedded_DFUHelperDeviceInfo_json,
        embedded_DFUHelperDeviceInfo_json +
        embedded_DFUHelperDeviceInfo_json_len
    );

    auto itMap = j["devicemap"].find(deviceKey);
    if (itMap == j["devicemap"].end())
        return seq;

    const std::string deviceType = itMap.value();

    auto itLayout = j["devicelayout"].find(deviceType);
    if (itLayout == j["devicelayout"].end())
        return seq;

    const auto& layout = itLayout.value();

    // assume supported if not specified
    // mainly for appletv4k
    // TODO: integrate libtvcontrol for breakout boards
    seq.isSupported = layout.value("is_supported", true);
    seq.imageName = layout.value("image_name", "");
    seq.imageWidth = layout.value("image_width", 0);
    seq.imageHeight = layout.value("image_height", 0);

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

bool SequenceIsSupported(const DfuSequence* seq)
{
    return seq && !seq->steps.empty();
}

bool SequenceRequiresCLI(const DfuSequence* seq)
{
    return seq && !seq->isSupported;
}

#endif // WITH_GUI || WITH_TUI
