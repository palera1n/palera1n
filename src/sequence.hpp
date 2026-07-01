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
    int imageOffsetX = 0;

    std::vector<DfuButton> buttons;
    std::vector<DfuStep> steps;
};

DfuSequence ParseSequence(const std::string& deviceKey);
bool SequenceIsSupported(const std::string& deviceKey);

#endif // SEQUENCE_H

#endif // WITH_GUI || WITH_TUI
