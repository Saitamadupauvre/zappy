from __future__ import annotations
from abc import ABC, abstractmethod
from player.context import BotContext


class State(ABC):
    @abstractmethod
    def enter(self, ctx: BotContext) -> None: ...

    @abstractmethod
    def update(self, ctx: BotContext) -> "State | None": ...

    @abstractmethod
    def exit(self, ctx: BotContext) -> None: ...


class FSM:
    def __init__(self, initial: State, ctx: BotContext) -> None:
        self._state = initial
        self._state.enter(ctx)

    @property
    def current(self) -> State:
        return self._state

    def tick(self, ctx: BotContext) -> None:
        next_state = self._state.update(ctx)
        if next_state is not None:
            self._state.exit(ctx)
            self._state = next_state
            self._state.enter(ctx)
