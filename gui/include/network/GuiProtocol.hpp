#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace zappy::net {

enum class MessageKind {
    Unknown,
    Msz,
    Bct,
    Mct,
    Tna,
    Pnw,
    Ppo,
    Plv,
    Pfk,
    Pin,
    Pex,
    Pbc,
    Pic,
    Pie,
    Pdr,
    Pgt,
    Pdi,
    Enw,
    Ebo,
    Edi,
    Sgt,
    Sst,
    Seg,
    Smg,
    Suc,
    Sbp,
};

struct Message {
    MessageKind kind{MessageKind::Unknown};
    std::string command;
    std::vector<std::string> args;
    std::string raw;
};

} // namespace zappy::net