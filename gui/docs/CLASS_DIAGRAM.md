# Class Diagram

## Core architecture

```mermaid
classDiagram
    direction TB

    class GameEngine {
        -World _world
        -CommandExecutor _executor
        -GuiNetworkManager _network
        -GraphicsContext _graphics
        -IScene _scene
        -AppState _state
        +run()
        +switchToMenu()
        +switchToGame()
        +startConnecting()
        +getStatus() int
    }

    class AppState {
        <<enumeration>>
        Menu
        Connecting
        InGame
    }

    class World {
        -Tile2D _grid
        -map~uint32 PlayerState~ _players
        -map~uint32 EggState~ _eggs
        -vector~string~ _teams
        +resize(w, h)
        +setTile(x, y, tile)
        +addPlayer(PlayerState)
        +movePlayer(id, x, y, o)
        +addEgg(EggState)
        +removeEgg(id)
        +getTile(x, y) Tile
        +getPlayer(id) PlayerState
        +getEgg(id) EggState
        +getPlayers() map
        +getTeams() vector
        +emit(WorldEvent)
        +setEventDispatcher(fn)
    }

    class CommandExecutor {
        -World _world
        -map~MessageKind ICommand~ _commandTable
        +execute(Message)
        -handleMapSize()
        -handleNewPlayer()
        -handlePlayerBroadcast()
        -handleTileContent()
    }

    class GuiNetworkManager {
        -GuiConnection _connection
        -CommandParser _parser
        -queue~Message~ _messages
        -string _buffer
        -HandshakeState _state
        +connect()
        +disconnect()
        +update()
        +sendLine(line)
        +tryPopCommand() Message
    }

    class GraphicsContext {
        -IWindowContext _window
        -IRenderer _renderer
        -IMeshFactory _meshFactory
        -ITextureLoader _textureLoader
        -IFontLoader _fontLoader
        +pollAndDispatch()
        +beginFrame()
        +endFrame()
        +isOpen() bool
        +setTargetFps(fps)
    }

    class ProcessSpawner {
        +spawn(cmd) pid
    }

    GameEngine *-- World
    GameEngine *-- CommandExecutor
    GameEngine *-- GuiNetworkManager
    GameEngine *-- GraphicsContext
    GameEngine *-- IScene
    GameEngine --> AppState
    CommandExecutor --> World : mutates + emits
    GuiNetworkManager --> CommandExecutor : dispatches parsed messages
```

## Scene hierarchy

```mermaid
classDiagram
    direction TB

    class IScene {
        <<interface>>
        +update(World, dt)*
        +render(IRenderer)*
        +handleEvent(Event)*
        +getHud() HudManager
        +getEntityManager() EntityManager
    }

    class Scene {
        <<abstract>>
        #EntityManager _entities
        #HudManager _hud
        #OrbitCamera _camera
        #InputManager _inputManager
        #HudPicker _hudPicker
        #PickSystem _pickSystem
        +render(IRenderer)
        +handleEvent(Event)
        +update(World, dt)*
    }

    class WorldScene {
        -World _world
        -SceneHudManager _hudMgr
        -PlayerTileSystem _tileSystem
        -WorldBuilder _worldBuilder
        -CameraController _camCtrl
        -PlayerEntityFactory _playerFactory
        -EggEntityFactory _eggFactory
        -int _worldW
        -int _worldH
        -uint32 _selectedPlayerId
        -function _sendLine
        -map~uint32 Color~ _teamColors
        +update(World, dt)
        +handleEvent(Event)
        +setSendLine(fn)
        +setSetFps(fn)
        -onWorldResized()
        -onPlayerAdded()
        -onPlayerMoved()
        -onEggAdded()
        -onBroadcast()
        -onEntitySelected()
    }

    class MenuScene {
        -MenuState _state
        -MainMenuPanel _mainMenu
        -ConnectPanel _connect
        -LaunchPanel _launch
        -ProcessSpawner _spawner
        +update(World, dt)
        +render(IRenderer)
        +handleEvent(Event)
        +setConnectStatus(status)
    }

    class MenuState {
        <<enumeration>>
        Main
        Connect
        Launch
    }

    class OrbitCamera {
        +float yaw
        +float pitch
        +float distance
        +float fov
        -Mode _mode
        +enterCenter()
        +enterFollow(pos)
        +enterFirstPerson(pos, fwd, up)
        +updateFollowTarget(pos)
        +setMapCenter(pos)
        +isFollowing() bool
        +toCameraState() CameraState
        +handleEvent(Event)
        +update(dt)
    }

    class CameraController {
        -OrbitCamera _camera
        -uint64 _lastFollowTap
        +onFollowToggle(playerId, entities, hud)
        +onEntitySelected(id, entities)
        +enterFirstPerson()
        +exitFirstPerson()
        +applyLayoutFraming()
    }

    IScene <|-- Scene
    Scene <|-- WorldScene
    Scene <|-- MenuScene
    Scene *-- OrbitCamera
    Scene *-- EntityManager
    Scene *-- HudManager
    Scene *-- InputManager
    Scene *-- HudPicker
    Scene *-- PickSystem
    WorldScene *-- CameraController
    WorldScene *-- SceneHudManager
    WorldScene *-- PlayerTileSystem
    WorldScene *-- WorldBuilder
    MenuScene --> MenuState
    CameraController --> OrbitCamera : wraps
```

## Entity & Behavior system

```mermaid
classDiagram
    direction TB

    class EntityManager {
        -vector~Entity~ _entities
        -map~EntityID Entity~ _entityIndex
        +addEntity(Entity)
        +createEntity(id, type) Entity
        +removeEntity(id)
        +getEntity(id) Entity
        +update(dt)
        +handleEvent(Event)
        +clear()
        -applySelection(id)
    }

    class Entity {
        -EntityID _id
        -string _type
        -vector~IBehaviorPtr~ _behaviors
        +addBehavior~T~(args) T
        +getBehavior~T~() T
        +hasBehavior~T~() bool
        +removeBehavior~T~()
        +update(dt)
        +handleEvent(Event)
    }

    class IBehavior {
        <<interface>>
        +onAttach(Entity)*
        +onUpdate(Entity, dt)*
        +onEvent(Entity, Event)*
    }

    class ABehavior {
        +onAttach(Entity)
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class ADrawable {
        +onEvent(Entity, Event)
        +draw(Entity, IRenderer)*
        +getMesh() MeshHandle
        +isVisible() bool
    }

    class ModelDrawableBehavior {
        -ModelHandle _modelHandle
        -Color4b _color
        -bool _ownsModel
        -vector~Color~ _meshTints
        +setTint(color)
        +setModel(handle)
        +setMeshTints(tints)
        +setRotationOffset(mat)
        +draw(Entity, IRenderer)
    }

    class MeshDrawableBehavior {
        -MeshHandle _meshHandle
        -Color4b _tint
        +setTint(color)
        +setVisible(bool)
        +draw(Entity, IRenderer)
    }

    class TextDrawableBehavior {
        -string _text
        -float _fontSize
        -Color _color
        +setText(text)
        +setAnchor(anchor)
        +draw(Entity, IRenderer)
    }

    class GroundDrawableBehavior {
        -MeshHandle _mesh
        -TextureHandle _texture
        -GrassFieldHandle _grass
        +setGrassEnabled(bool)
        +draw(Entity, IRenderer)
    }

    class HudContainerBehavior {
        -IHudProvider _provider
        -LayoutEngine _layoutEngine
        -AnimState _animState
        -float _uiScale
        -float _scrollOffsetY
        -float _contentHeight
        -vector _buttonAreas
        -vector _sliderAreas
        -vector _inputAreas
        +setProvider(provider)
        +setVisible(bool, duration)
        +isVisible() bool
        +isFullyVisible() bool
        +setScrollable(bool)
        +scrollToBottom()
        +setTitle(text, size)
        +draw(Entity, IRenderer)
        +onEvent(Entity, Event)
        +onUpdate(Entity, dt)
    }

    class AnimState {
        <<enumeration>>
        Hidden
        FadingIn
        Visible
        FadingOut
    }

    class TransformBehavior {
        -Vector3f _position
        -Vector3f _rotation
        -float _scale
        -Matrix _customRot
        +setPosition(pos)
        +setRotation(rot)
        +setScale(s)
        +setOrientation(mat)
        +getMatrix() Matrix
    }

    class MovementBehavior {
        -Vector3f _startPos
        -Vector3f _endPos
        -float _elapsed
        -float _duration
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class RotationBehavior {
        -float _startYaw
        -float _targetYaw
        -Vector3f _startUp
        -Vector3f _targetUp
        -Vector3f _startFwd
        -Vector3f _targetFwd
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class SelectableBehavior {
        -bool _selected
        -SelectCallback _cb
        +setSelected(bool)
        +toggle()
        +onEvent(Entity, Event)
    }

    class OutlineBehavior {
        -bool _isSelected
        -bool _isHovered
        +setHovered(bool)
        +setSelected(bool)
        +drawOutline(Entity, IRenderer)
        +onEvent(Entity, Event)
    }

    class ClickableBehavior {
        -ClickCallback _onClick
        +setOnClick(fn)
        +onEvent(Entity, Event)
    }

    class HoverableBehavior {
        -function _onEnter
        -function _onLeave
        +setHovered(bool)
        +onEvent(Entity, Event)
    }

    class TagBehavior {
        -EntityID _tagEntityId
        -float _offsetY
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    EntityManager *-- "0..*" Entity
    Entity *-- "0..*" IBehavior
    IBehavior <|-- ABehavior
    ABehavior <|-- ADrawable
    ABehavior <|-- TransformBehavior
    ABehavior <|-- MovementBehavior
    ABehavior <|-- RotationBehavior
    ABehavior <|-- SelectableBehavior
    ABehavior <|-- OutlineBehavior
    ABehavior <|-- ClickableBehavior
    ABehavior <|-- HoverableBehavior
    ABehavior <|-- TagBehavior
    ADrawable <|-- ModelDrawableBehavior
    ADrawable <|-- MeshDrawableBehavior
    ADrawable <|-- TextDrawableBehavior
    ADrawable <|-- GroundDrawableBehavior
    ADrawable <|-- HudContainerBehavior
    HudContainerBehavior --> AnimState
```

## Player-specific behaviors

```mermaid
classDiagram
    direction TB

    class PlayerBehavior {
        -uint32 _playerId
        -int _x
        -int _y
        -int _orientation
        -int _level
        -string _team
        -Resources _inventory
        +playerId() uint32
        +x() int
        +y() int
        +orientation() int
        +level() int
        +team() string
        +inventory() Resources
    }

    class PlayerAnimationBehavior {
        -int _frame
        -int _animIndex
        -float _moveDuration
        -bool _running
        -bool _takePlaying
        -bool _dancing
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class PlayerLevelModelBehavior {
        -ModelHandle[8] _levelModels
        -vector[8] _skinMeshIndices
        +onEvent(Entity, Event)
    }

    class BroadcastBehavior {
        -float _timeLeft
        -bool _active
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class ResourceBehavior {
        -int _x
        -int _y
        -int _type
        -int _count
        +getX() int
        +getY() int
        +getType() int
        +getCount() int
        +onEvent(Entity, Event)
    }

    class EggBehavior {
        -float _time
        -bool _pendingRemoval
        -float _removeTimer
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class RitualCircleBehavior {
        -MeshHandle _mesh
        -Vector3f _center
        -Vector3f _normal
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class IncantationTileBehavior {
        -float _time
        -float _flashTimer
        -bool _active
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class ExplosionBehavior {
        -Phase _phase
        -vector~Spark~ _sparks
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    class WaveBroadcastBehavior {
        -float _radius
        -float _maxRadius
        -bool _active
        +onUpdate(Entity, dt)
        +onEvent(Entity, Event)
    }

    ABehavior <|-- PlayerBehavior
    ABehavior <|-- PlayerAnimationBehavior
    ABehavior <|-- PlayerLevelModelBehavior
    ABehavior <|-- BroadcastBehavior
    ABehavior <|-- ResourceBehavior
    ABehavior <|-- EggBehavior
    ABehavior <|-- RitualCircleBehavior
    ABehavior <|-- IncantationTileBehavior
    ABehavior <|-- ExplosionBehavior
    ABehavior <|-- WaveBroadcastBehavior
```

## HUD provider pattern

```mermaid
classDiagram
    direction TB

    class IHudProvider {
        <<interface>>
        +getHudElements() vector~HudElement~*
        +getVersion() uint64
    }

    class HudElement {
        -HudElementData _data
    }

    class HudElementData {
        <<variant>>
        +TextData
        +BarData
        +RectData
        +ButtonData
        +ChatBubbleData
        +ImageData
        +SliderData
        +SlotData
    }

    class PlayerInfoProvider {
        -PlayerState _playerState
        +getHudElements() vector~HudElement~
    }

    class InventoryProvider {
        -Resources _inventory
        +getHudElements() vector~HudElement~
    }

    class TeamLeaderboardProvider {
        -TeamLeaderboardStore _store
        +getHudElements() vector~HudElement~
    }

    class TeamChatProvider {
        -TeamChatStore _store
        +getHudElements() vector~HudElement~
    }

    class SpeedControlProvider {
        -int _currentSpeed
        -function _onRelease
        -function _onChange
        +getHudElements() vector~HudElement~
    }

    class NotificationPopupProvider {
        -string _icon
        -string _title
        -string _subtitle
        +getHudElements() vector~HudElement~
    }

    class PlayerTagProvider {
        -string _name
        -Color _teamColor
        -bool _isBroadcasting
        +getHudElements() vector~HudElement~
    }

    class ResourceInfoProvider {
        -Tile _tile
        +getHudElements() vector~HudElement~
    }

    class LeaderboardControlProvider {
        -int _page
        -function _onPrev
        -function _onNext
        -function _onWorldInfo
        +getHudElements() vector~HudElement~
    }

    class TeamDetailProvider {
        -vector _players
        -function _onFollowClick
        +getHudElements() vector~HudElement~
    }

    class SettingsProvider {
        -vector _sections
        +getHudElements() vector~HudElement~
    }

    class ClockProvider {
        -float _uptimeSeconds
        -int _timeUnit
        +getHudElements() vector~HudElement~
    }

    class WorldInfoProvider {
        -World _world
        +getHudElements() vector~HudElement~
    }

    class MainMenuProvider {
        -function _onConnect
        -function _onLaunch
        -function _onQuit
        +getHudElements() vector~HudElement~
    }

    class ConnectProvider {
        -string _host
        -string _port
        -string _status
        -function _onConnect
        +getHudElements() vector~HudElement~
    }

    class LaunchProvider {
        -function _onLaunch
        +getHudElements() vector~HudElement~
    }

    IHudProvider <|-- PlayerInfoProvider
    IHudProvider <|-- InventoryProvider
    IHudProvider <|-- TeamLeaderboardProvider
    IHudProvider <|-- TeamChatProvider
    IHudProvider <|-- SpeedControlProvider
    IHudProvider <|-- NotificationPopupProvider
    IHudProvider <|-- PlayerTagProvider
    IHudProvider <|-- ResourceInfoProvider
    IHudProvider <|-- LeaderboardControlProvider
    IHudProvider <|-- TeamDetailProvider
    IHudProvider <|-- SettingsProvider
    IHudProvider <|-- ClockProvider
    IHudProvider <|-- WorldInfoProvider
    IHudProvider <|-- MainMenuProvider
    IHudProvider <|-- ConnectProvider
    IHudProvider <|-- LaunchProvider
    HudContainerBehavior --> IHudProvider : calls each frame
    IHudProvider --> HudElement : returns
    HudElement --> HudElementData : wraps
```

## HUD panels

```mermaid
classDiagram
    direction TB

    class SceneHudManager {
        -LeaderboardPanel _leaderboard
        -TeamDetailPanel _teamDetail
        -ChatPanel _chat
        -PlayerInfoPanel _playerInfo
        -PopupPanel _popup
        -ResourceInfoPanel _resourceInfo
        -SpeedPanel _speed
        -SettingsPanel _settings
        -InventoryPanel _inventory
        -ClockPanel _clock
        -TeamStatsPanel _teamStats
        -WorldInfoPanel _worldInfo
        +setup(hud, entities, ...)
        +onTeamAdded(team)
        +onPlayerAdded(player)
        +onPlayerLevelChanged(id, level)
        +onBroadcast(id, msg)
        +onEntitySelected(id)
    }

    class LeaderboardPanel {
        -TeamLeaderboardStore _lbStore
        -map _providers
        -map _containers
        +setup(hud, entities)
        +onTeamAdded(team)
        +onPlayerAdded(player)
        +onPlayerLevelChanged(id, level)
        +toggle()
        +setOnDetailsClick(fn)
        +setVotedTeam(team)
    }

    class TeamDetailPanel {
        -TeamDetailProvider _provider
        +setup(hud)
        +open(team, players)
        +refreshIfOpen(players)
        +close()
        +setOnFollowClick(fn)
    }

    class PlayerInfoPanel {
        -PlayerInfoProvider _provider
        +setup(hud)
        +show(player)
        +clear()
        +setOnChatClick(fn)
        +setVotedTeam(team)
    }

    class ChatPanel {
        -TeamChatStore _chatStore
        -TeamChatProvider _provider
        +setup(hud)
        +open(team)
        +close()
        +onBroadcast(id, team, msg)
        +setPlayerTeam(id, team)
        +removePlayer(id)
    }

    class InventoryPanel {
        -InventoryProvider _provider
        -uint32 _selectedPlayerId
        +setup(hud)
        +show()
        +hide()
        +toggle()
        +setSelectedPlayer(id)
        +onInventoryChanged(id, inv)
    }

    class PopupPanel {
        -NotificationPopupProvider[3] _providers
        +setup(hud)
        +push(icon, title, subtitle)
        +tick(dt)
    }

    class SpeedPanel {
        -SpeedControlProvider _provider
        +setup(hud)
        +setSendLine(fn)
        +setTimeUnit(tu)
    }

    class SettingsPanel {
        -SettingsProvider _provider
        +setup(hud, inputMgr)
        +toggle()
    }

    class ResourceInfoPanel {
        -ResourceInfoProvider _provider
        +setup(hud)
        +show(tile)
        +hide()
    }

    class ClockPanel {
        -ClockProvider _provider
        +setup(hud)
        +setUptime(s)
        +setTimeUnit(tu)
        +tick(dt)
    }

    class TeamStatsPanel {
        +setup(hud)
        +show(team)
        +hide()
        +isVisible() bool
    }

    class WorldInfoPanel {
        -WorldInfoProvider _provider
        +setup(hud)
        +show()
        +hide()
        +toggle()
    }

    SceneHudManager *-- LeaderboardPanel
    SceneHudManager *-- TeamDetailPanel
    SceneHudManager *-- PlayerInfoPanel
    SceneHudManager *-- ChatPanel
    SceneHudManager *-- InventoryPanel
    SceneHudManager *-- PopupPanel
    SceneHudManager *-- SpeedPanel
    SceneHudManager *-- SettingsPanel
    SceneHudManager *-- ResourceInfoPanel
    SceneHudManager *-- ClockPanel
    SceneHudManager *-- TeamStatsPanel
    SceneHudManager *-- WorldInfoPanel
```

## Entity builder pattern

```mermaid
classDiagram
    direction TB

    class EntityBuilder {
        -EntityManager _mgr
        -EntityID _id
        -string _type
        +transform() TransformBuilder
        +movement() MovementBuilder
        +interaction() InteractionBuilder
        +drawable() DrawableBuilder
        +player() PlayerBuilder
        +resource() ResourceBuilder
        +egg() EggBuilder
        +tag() TagBuilder
        +hud() HudBuilder
        +build() EntityPtr
    }

    class TransformBuilder {
        +position(pos) TransformBuilder
        +rotation(rot) TransformBuilder
        +scale(s) TransformBuilder
        +orientation(mat) TransformBuilder
    }

    class MovementBuilder {
        +move(from, to, duration) MovementBuilder
        +rotate(from, to, duration) MovementBuilder
        +animated(bool) MovementBuilder
    }

    class InteractionBuilder {
        +outline() InteractionBuilder
        +onClick(fn) InteractionBuilder
        +onHover(enter, leave) InteractionBuilder
        +selectable() InteractionBuilder
        +selectableOutline() InteractionBuilder
    }

    class DrawableBuilder {
        +mesh(handle, color) DrawableBuilder
        +model(renderer, handle, color, rot) DrawableBuilder
        +text(text, size, color) DrawableBuilder
        +meshVisible(bool) DrawableBuilder
        +textVisible(bool) DrawableBuilder
    }

    class PlayerBuilder {
        +state(playerState) PlayerBuilder
        +broadcast() PlayerBuilder
        +animation(anims) PlayerBuilder
        +levelModel(models) PlayerBuilder
    }

    class HudBuilder {
        +container(provider) HudBuilder
        +layout(type, padding) HudBuilder
        +anchor(anchor) HudBuilder
        +anchorOffset(offset) HudBuilder
        +background(show, fill, border) HudBuilder
        +boxSize(size) HudBuilder
        +autoSize() HudBuilder
        +title(text, size) HudBuilder
        +isWorldSpaceTag(bool) HudBuilder
        +hidden() HudBuilder
    }

    EntityBuilder *-- TransformBuilder
    EntityBuilder *-- MovementBuilder
    EntityBuilder *-- InteractionBuilder
    EntityBuilder *-- DrawableBuilder
    EntityBuilder *-- PlayerBuilder
    EntityBuilder *-- HudBuilder
```

## Factory system

```mermaid
classDiagram
    direction TB

    class PlayerEntityFactory {
        -ModelHandle[8] _levelModels
        -map~uint32 PlayerTagProvider~ _tagProviders
        -float MODEL_SCALE
        +init(renderer)
        +spawn(player, entityMgr, hudMgr, layout)
        +assignTeamColor(team, color)
        +setPlayerTeamColor(id, team)
        +removePlayer(id)
    }

    class EggEntityFactory {
        -ModelHandle _model
        +init(renderer)
        +spawn(egg, entityMgr, layout)
        +clearAll(entityMgr)
    }

    class ResourceEntityFactory {
        -ModelHandle _model
        -ModelHandle _foodModel
        +init(renderer)
        +spawnAll(tile, x, y, entityMgr, layout)
        +clearAll(entityMgr)
    }

    class TileEntityFactory {
        +init(renderer)
        +spawn(x, y, entityMgr, layout)
    }

    class MapGroundFactory {
        +build(w, h, layout, entityMgr, meshFactory, renderer)
        +clear(entityMgr)
    }

    class WorldBuilder {
        -ResourceEntityFactory _resourceFactory
        -MapGroundFactory _groundFactory
        +init(renderer, meshFactory)
        +build(world, entityMgr, layout)
        +rebuildGroundOnly(w, h, entityMgr, layout)
        +clear(entityMgr)
    }

    WorldBuilder *-- ResourceEntityFactory
    WorldBuilder *-- MapGroundFactory
    WorldScene *-- PlayerEntityFactory
    WorldScene *-- EggEntityFactory
    WorldScene *-- WorldBuilder
```

## Map layout system

```mermaid
classDiagram
    direction TB

    class IMapLayout {
        <<interface>>
        +tilePos(x, y) Vector3f*
        +standY(x, y) float*
        +upAt(x, y) Vector3f*
        +forwardAt(x, y) Vector3f*
        +buildMesh(w, h, factory) MeshHandle*
        +cameraFraming(w, h) CameraFraming*
        +animatesWrap() bool*
        +updateSizing(w, h)*
    }

    class GridLayout {
        -float _spacing
        -float _groundY
        +tilePos(x, y) Vector3f
        +standY(x, y) float
        +upAt(x, y) Vector3f
        +forwardAt(x, y) Vector3f
        +buildMesh(w, h, factory) MeshHandle
        +cameraFraming(w, h) CameraFraming
        +animatesWrap() bool
        +updateSizing(w, h)
    }

    class TorusLayout {
        -float _R
        -float _r
        -float _tileArcW
        -float _tileArcH
        +tilePos(x, y) Vector3f
        +standY(x, y) float
        +upAt(x, y) Vector3f
        +forwardAt(x, y) Vector3f
        +buildMesh(w, h, factory) MeshHandle
        +cameraFraming(w, h) CameraFraming
        +animatesWrap() bool
        +updateSizing(w, h)
    }

    class SphereLayout {
        -float _radius
        +tilePos(x, y) Vector3f
        +standY(x, y) float
        +upAt(x, y) Vector3f
        +forwardAt(x, y) Vector3f
        +buildMesh(w, h, factory) MeshHandle
        +cameraFraming(w, h) CameraFraming
        +animatesWrap() bool
        +updateSizing(w, h)
    }

    IMapLayout <|-- GridLayout
    IMapLayout <|-- TorusLayout
    IMapLayout <|-- SphereLayout
    WorldScene --> IMapLayout : owns active layout
```

## Player tile & world systems

```mermaid
classDiagram
    direction TB

    class PlayerTileSystem {
        -map~uint32 TileCoord~ _playerTiles
        -map~uint32 int~ _playerOrient
        +setActiveLayout(layout)
        +onPlayerAdded(player, entityMgr)
        +onPlayerMoved(id, x, y, o, entityMgr, clock)
        +onPlayerRemoved(id)
        +repositionAll(entityMgr, layout)
    }

    class AnimationClock {
        -float _timeUnit
        +setTimeUnit(tu)
        +toSeconds(ticks) float
        +moveDuration() float
        +incantationDuration() float
    }

    class TeamLeaderboardStore {
        -map _teams
        -map _players
        -vector _rankedCache
        -map _teamDetailCache
        +addTeam(name)
        +setPlayerTeam(id, team)
        +updateLevel(id, level)
        +getRankedTeams() vector~TeamStat~
        +getPlayersForTeam(team) vector~PlayerDetailStat~
    }

    class TeamChatStore {
        -map~string vector~Message~~ _messages
        -map~uint32 string~ _playerTeams
        +addMessage(playerId, text)
        +getTeamMessages(team) vector~Message~
        +setPlayerTeam(id, team)
        +removePlayer(id)
    }

    class GrassBuilder {
        +build(params, meshFactory) GrassField
        +makeBladeMesh(meshFactory) MeshHandle
    }

    WorldScene *-- PlayerTileSystem
    WorldScene *-- AnimationClock
    LeaderboardPanel *-- TeamLeaderboardStore
    ChatPanel *-- TeamChatStore
```

## Event system

```mermaid
classDiagram
    direction LR

    class Event {
        <<variant>>
        +WindowEvent
        +RenderEvent
        +WorldEvent
        +LogicEvent
    }

    class WorldEvent {
        <<variant>>
        +PlayerAddedEvent
        +PlayerMovedEvent
        +PlayerRemovedEvent
        +PlayerBroadcastEvent
        +PlayerLevelChangedEvent
        +PlayerInventoryEvent
        +TileChangedEvent
        +IncantationStartEvent
        +IncantationEndEvent
        +EggAddedEvent
        +EggHatchedEvent
        +EggDeadEvent
        +GameEndedEvent
        +ServerTimeEvent
        +MapSizeEvent
        +TeamAddedEvent
    }

    class LogicEvent {
        <<variant>>
        +EntityMoveToEvent
        +EntityRotateToEvent
        +SelectEvent
        +EntitySelectedEvent
        +ClickEvent
        +HoverEvent
        +CameraFollowToggleEvent
    }

    class WindowEvent {
        <<variant>>
        +CloseEvent
        +ResizeEvent
        +MouseButtonEvent
        +MouseMoveEvent
        +MouseWheelEvent
        +KeyEvent
    }

    class RenderEvent {
        <<variant>>
        +Draw3DEvent
        +Draw2DEvent
    }

    class on_function {
        <<function template>>
        +on(Event, handlers...)
    }

    Event *-- WorldEvent
    Event *-- LogicEvent
    Event *-- WindowEvent
    Event *-- RenderEvent
    on_function --> Event : recurses nested variants
```

## Network pipeline

```mermaid
classDiagram
    direction LR

    class Socket {
        -int _fd
        +connect(host, port)
        +read(buf) int
        +writeAll(data)
        +writeSome(data) int
        +listen(port)
        +accept() Socket
    }

    class PollManager {
        -vector~PollEntry~ _entries
        -map _callbacks
        +addSocket(fd, events, cb)
        +updateSocket(fd, events)
        +removeSocket(fd)
        +pollLoop()
    }

    class GuiConnection {
        -Socket _socket
        -string _incoming
        -queue~string~ _writeQueue
        +connectToServer(host, port)
        +processReadable()
        +processWritable()
        +queueLine(line)
        +drainIncoming() vector~string~
    }

    class GuiNetworkManager {
        -GuiConnection _connection
        -CommandParser _parser
        -HandshakeState _state
        -queue~Message~ _messages
        +connect(host, port)
        +disconnect()
        +update()
        +sendLine(line)
        +tryPopCommand() optional~Message~
    }

    class CommandParser {
        -map~string MessageKind~ _stringToKind
        +parseLine(line) Message
    }

    class CommandExecutor {
        -World _world
        -map~MessageKind ICommand~ _commandTable
        +execute(Message)
    }

    class ICommand {
        <<interface>>
        +execute(Message)*
    }

    class Command {
        -CommandCallback _cb
        +execute(Message)
    }

    Socket <-- GuiConnection : owns
    PollManager <-- GuiConnection : uses
    GuiConnection <-- GuiNetworkManager : owns
    CommandParser <-- GuiNetworkManager : owns
    GuiNetworkManager --> CommandExecutor : calls execute()
    CommandExecutor *-- "0..*" ICommand
    ICommand <|-- Command
```

## Graphics abstraction

```mermaid
classDiagram
    direction TB

    class IRenderer {
        <<interface>>
        +beginFrame()*
        +endFrame()*
        +begin3D(camera)*
        +end3D()*
        +begin2D()*
        +end2D()*
        +drawMesh(handle, transform, color)*
        +drawModel(handle, transform, color)*
        +drawText(text, pos, size, color)*
        +loadModel(path) ModelHandle*
        +loadTexture(path) TextureHandle*
        +loadFont(path) FontHandle*
        +loadShader(vs, fs) ShaderHandle*
        +drawGrassField(field, camera)*
        +drawSkybox(handle)*
    }

    class IWindowContext {
        <<interface>>
        +shouldClose() bool*
        +getFrameTime() float*
        +getViewport() Vector2f*
        +pollEvents()*
        +setTargetFps(fps)*
        +setFullscreen(bool)*
    }

    class IMeshFactory {
        <<interface>>
        +createPlane(w, h) MeshHandle*
        +createCube(size) MeshHandle*
        +createSphere(r, rings, slices) MeshHandle*
        +createTorus(r, R, sides, rings) MeshHandle*
        +createCylinder(r, h, slices) MeshHandle*
    }

    class ITextureLoader {
        <<interface>>
        +loadFromFile(path) TextureHandle*
        +loadFromMemory(data, size) TextureHandle*
    }

    class IFontLoader {
        <<interface>>
        +loadFromFile(path) FontHandle*
        +loadFromMemory(data, size) FontHandle*
    }

    class RaylibRenderer {
        -map _models
        -map _textures
        -map _fonts
        -map _shaders
        -map _grassFields
        -map _skyboxes
        +beginFrame()
        +endFrame()
        +begin3D(camera)
        +end3D()
        +begin2D()
        +end2D()
        +drawMesh(handle, transform, color)
        +drawModel(handle, transform, color)
        +drawText(text, pos, size, color)
        +loadModel(path) ModelHandle
        +loadTexture(path) TextureHandle
        +drawGrassField(field, camera)
        +drawSkybox(handle)
    }

    class RaylibWindow {
        -RaylibWindowHandle _handle
        +shouldClose() bool
        +getFrameTime() float
        +getViewport() Vector2f
        +pollEvents()
        +setTargetFps(fps)
        +setFullscreen(bool)
    }

    class RaylibMeshFactory {
        +createPlane(w, h) MeshHandle
        +createCube(size) MeshHandle
        +createSphere(r, rings, slices) MeshHandle
        +createTorus(r, R, sides, rings) MeshHandle
        +createCylinder(r, h, slices) MeshHandle
    }

    class RaylibTextureLoader {
        +loadFromFile(path) TextureHandle
        +loadFromMemory(data, size) TextureHandle
    }

    class RaylibFontLoader {
        +loadFromFile(path) FontHandle
        +loadFromMemory(data, size) FontHandle
    }

    IRenderer <|-- RaylibRenderer
    IWindowContext <|-- RaylibWindow
    IMeshFactory <|-- RaylibMeshFactory
    ITextureLoader <|-- RaylibTextureLoader
    IFontLoader <|-- RaylibFontLoader
    GraphicsContext *-- IWindowContext
    GraphicsContext *-- IRenderer
    GraphicsContext *-- IMeshFactory
    GraphicsContext *-- ITextureLoader
    GraphicsContext *-- IFontLoader
```

## Input & picking system

```mermaid
classDiagram
    direction TB

    class InputManager {
        -map~InputAction Key~ _keyBindings
        -map~InputAction bool~ _actionStates
        -vector _listeners
        +handleEvent(Event)
        +bindActionListener(action, fn)
        +rebindAction(action, key)
        +captureNextKey(fn)
        +isActionActive(action) bool
    }

    class InputAction {
        <<enumeration>>
        MOVE_FORWARD
        MOVE_BACKWARD
        MOVE_LEFT
        MOVE_RIGHT
        TOGGLE_POV
        FOLLOW_TOGGLE
        CYCLE_LAYOUT
        TOGGLE_TILE_SHADING
        TOGGLE_LEADERBOARD
        TOGGLE_SETTINGS
        TOGGLE_CHAT
    }

    class HudPicker {
        -HudManager _hud
        +tryHandleMouseMove(pos) bool
        +tryHandleClick(pos, btn) bool
        +isWheelConsumed(pos) bool
    }

    class PickSystem {
        -EntityManager _entities
        -IRenderer _renderer
        +tryHandleClick(ray, ev) bool
        +tryHandleMouseMove(ray, ev) bool
        +raycast(ray) optional~EntityID~
        -spherePreCull(ray, entity) bool
    }

    class LayoutEngine {
        -Type _type
        -float _padding
        +calculate(elements, boxSize) vector~Rect~
    }

    class LayoutType {
        <<enumeration>>
        Vertical
        Horizontal
        MediaObject
        VerticalMedia
        Grid
        MediaObjectHButtons
    }

    InputManager --> InputAction
    LayoutEngine --> LayoutType
    Scene *-- InputManager
    Scene *-- HudPicker
    Scene *-- PickSystem
```

## Logging & service locator

```mermaid
classDiagram
    direction TB

    class Logger {
        -map _sinks
        +addSink(sink, level)
        +log(level, msg)
        +debug(args...)
        +info(args...)
        +warn(args...)
        +error(args...)
        +setMinLevel(level)
    }

    class ContextLogger {
        -string _origin
        -Logger _logger
        +debug(args...)
        +info(args...)
        +warn(args...)
        +error(args...)
        +trace(args...)
    }

    class LogLevel {
        <<enumeration>>
        TRACE
        DEBUG
        INFO
        WARNING
        ERROR
        NONE
    }

    class Locator {
        <<static>>
        -Logger* _logger
        -IScene* _scene
        -IRenderer* _renderer
        -FontHandle _defaultFont
        +provide(Logger*)$
        +provide(IScene*)$
        +provide(IRenderer*)$
        +provideDefaultFont(handle)$
        +getLogger() Logger*$
        +getScene() IScene*$
        +getRenderer() IRenderer*$
        +getDefaultFont() FontHandle$
    }

    Logger --> LogLevel
    ContextLogger --> Logger : delegates
    Locator --> Logger : stores ptr
    Locator --> IScene : stores ptr
    Locator --> IRenderer : stores ptr
```
