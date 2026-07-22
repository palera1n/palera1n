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

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <string>
#include <vector>

struct DfuButton
{
    std::string id;
    std::string name;
    int x = 0;
    int y = 0;
};

struct DfuStep
{
    std::string description;
    int duration = 0;
    std::vector<std::string> activeButtons;
    std::string action;
};

struct DfuSequence
{
    std::string imageName;
    int imageWidth = 0;
    int imageHeight = 0;

    std::vector<DfuButton> buttons;
    std::vector<DfuStep> steps;
};

DfuSequence ParseSequence(const std::string& deviceKey);
bool SequenceIsSupported(const std::string& deviceKey);

#endif // SEQUENCE_H

#endif // WITH_GUI || WITH_TUI
