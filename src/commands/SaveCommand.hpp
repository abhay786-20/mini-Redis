#pragma once
#include "ICommand.hpp"

class SaveCommand : public ICommand {
public:
    SaveCommand() = default;
    std::string execute() override;
};
