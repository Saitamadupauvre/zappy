# Network & Protocol

## Flow

```
TCP socket
  └── GuiNetworkManager (poll loop, line buffering)
        └── CommandParser (line → net::Message{kind, args})
              └── CommandExecutor (dispatches to command handlers)
                    └── World::emit(WorldEvent) → WorldScene::handleEvent
```

`GuiNetworkManager` runs a non-blocking poll. It buffers incoming bytes until a `\n` is found, then forwards the complete line to `CommandParser`.

## Protocol messages

Defined in `include/network/GuiProtocol.hpp` as `net::MessageKind`:

| Kind | Command | Description |
|---|---|---|
| `Msz` | `msz X Y` | Map dimensions |
| `Bct` | `bct X Y r1..r7` | Tile contents |
| `Mct` | `mct ...` | Full map contents (series of bct) |
| `Tna` | `tna NAME` | Team name |
| `Pnw` | `pnw ID X Y O L T` | New player |
| `Ppo` | `ppo ID X Y O` | Player position |
| `Plv` | `plv ID L` | Player level |
| `Pin` | `pin ID X Y r1..r7` | Player inventory |
| `Pfk` | `pfk ID` | Player fork (egg laid) |
| `Pex` | `pex ID` | Player expulsion |
| `Pbc` | `pbc ID MSG` | Player broadcast |
| `Pic` | `pic X Y L ID...` | Incantation start |
| `Pie` | `pie X Y R` | Incantation end |
| `Pdr` | `pdr ID R` | Player dropped resource |
| `Pgt` | `pgt ID R` | Player collected resource |
| `Pdi` | `pdi ID` | Player death |
| `Enw` | `enw EID PID X Y` | New egg |
| `Ebo` | `ebo EID` | Egg hatched |
| `Edi` | `edi EID` | Egg died |
| `Sgt` | `sgt T` | Time unit (server → gui) |
| `Sst` | `sst T` | Time unit set (gui → server) |
| `Seg` | `seg T` | End of game |
| `Smg` | `smg MSG` | Server message |
| `Suc` | `suc` | Server uptime |
| `Sbp` | `sbp` | Bad parameters |
| `Stu` | `stu` | Unknown command |

## Sending commands to the server

From `WorldScene` (or any panel that has access to the `_sendLine` callback):

```cpp
_sendLine("sst 10");   // set game speed to 10
_sendLine("sgt");      // request current time unit
```

From a HUD panel: receive `_sendLine` as a `std::function<void(std::string)>` in your panel's setup and store it.

```cpp
// In WorldScene constructor / setup:
_hudMgr.speedPanel().setOnSpeedChange([this](int t) {
    _sendLine("sst " + std::to_string(t));
});
```

Never call `GuiNetworkManager` directly from scene or HUD code.

## Adding a new server command

### 1. Add to `MessageKind`

In `include/network/GuiProtocol.hpp`, add a new enum value:

```cpp
enum class MessageKind {
    // ...existing...
    MyCmd,
};
```

### 2. Register in CommandParser

In `src/parser/CommandParser/CommandParser.cpp`, add the mapping from string to kind:

```cpp
{"mycmd", net::MessageKind::MyCmd},
```

### 3. Add a WorldEvent (if needed)

In `include/event/WorldEvent.hpp`:

```cpp
struct MyCmdEvent { uint32_t id; std::string data; };

using WorldEvent = std::variant<
    // ...existing...
    MyCmdEvent,
>;
```

### 4. Handle in CommandExecutor

In `src/core/executor/CommandExecutor.cpp`, add a handler:

```cpp
// In initCommands():
_commandTable[net::MessageKind::MyCmd] = std::make_unique<MyCommand>(*this);

// Or inline with a lambda command:
void CommandExecutor::handleMyCmd(const std::string& arg) {
    _world.emit(event::WorldEvent{event::MyCmdEvent{/* parse args */}});
}
```

### 5. Handle in WorldScene

```cpp
void WorldScene::handleEvent(const event::Event& ev) {
    event::on(ev,
        // ...existing handlers...
        [&](const event::MyCmdEvent& e) { onMyCmd(e); }
    );
}
```

## Command pattern

Each protocol handler can be a class implementing `ICommand` (`include/parser/ICommand.hpp`):

```cpp
class MyCommand : public ICommand {
public:
    explicit MyCommand(CommandExecutor& exec) : _exec(exec) {}
    void execute(const net::Message& msg) override {
        if (msg.args.size() < 1) return;
        _exec.handleMyCmd(msg.args[0]);
    }
private:
    CommandExecutor& _exec;
};
```

Register it in `CommandExecutor::initCommands()`.
