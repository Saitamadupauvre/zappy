#pragma once

namespace zappy {

enum class ParseResult {
    NoMatch,
    Matched,
    Exit,
    Error
};

} // namespace zappy