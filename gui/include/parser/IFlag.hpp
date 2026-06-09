#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include "parser/flag/ParseResult.hpp"

namespace zappy {

class IFlag
{
    public:
        virtual ~IFlag() = default;
        [[nodiscard]] virtual bool isMandatory() const noexcept = 0;
        [[nodiscard]] virtual bool isSet() const noexcept = 0;
        [[nodiscard]] virtual ParseResult tryParse(std::vector<std::string>::const_iterator& it, 
                                                  std::vector<std::string>::const_iterator end) = 0;
        [[nodiscard]] virtual const std::string& getShortName() const = 0;
};

class FlagException : public std::runtime_error {
    public:
        explicit FlagException(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace zappy