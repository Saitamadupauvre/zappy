#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace zappy {

struct TeamStat {
    std::string name;
    int         maxLevel;
    int         playerCount;
};

class TeamLeaderboardStore {
public:
    void addTeam(const std::string& name)
    {
        if (_teams.find(name) == _teams.end()) {
            _teams[name] = {};
            ++_storeVersion;
            _rankDirty = true;
        }
    }

    void setPlayerTeam(uint32_t id, const std::string& team)
    {
        _playerTeam[id] = team;
        _playerLevel.emplace(id, 1);
        _teams[team].insert(id);
        recomputeMax(team);
    }

    void removePlayer(uint32_t id)
    {
        auto it = _playerTeam.find(id);
        if (it == _playerTeam.end()) return;
        const std::string& team = it->second;
        _teams[team].erase(id);
        _playerLevel.erase(id);
        recomputeMax(team);
        _playerTeam.erase(it);
    }

    void updateLevel(uint32_t id, int level)
    {
        _playerLevel[id] = level;
        auto it = _playerTeam.find(id);
        if (it != _playerTeam.end())
            recomputeMax(it->second);
    }

    const std::vector<TeamStat>& getRankedTeams() const
    {
        if (!_rankDirty) return _rankedCache;
        _rankedCache.clear();
        _rankedCache.reserve(_teams.size());
        for (const auto& [name, players] : _teams) {
            int mx = 0;
            for (uint32_t pid : players) {
                auto lit = _playerLevel.find(pid);
                if (lit != _playerLevel.end() && lit->second > mx)
                    mx = lit->second;
            }
            _rankedCache.push_back({name, mx, static_cast<int>(players.size())});
        }
        std::sort(_rankedCache.begin(), _rankedCache.end(), [](const TeamStat& a, const TeamStat& b) {
            if (a.maxLevel != b.maxLevel) return a.maxLevel > b.maxLevel;
            return a.name < b.name;
        });
        _rankDirty = false;
        return _rankedCache;
    }

    bool hasTeam(const std::string& name) const { return _teams.find(name) != _teams.end(); }

    uint64_t getVersion() const { return _storeVersion; }

    struct PlayerDetailStat { uint32_t id; int level; };

    const std::vector<PlayerDetailStat>& getPlayersForTeam(const std::string& team) const
    {
        if (_detailDirty) {
            _teamDetailCache.clear();
            _detailDirty = false;
        }
        auto dit = _teamDetailCache.find(team);
        if (dit != _teamDetailCache.end()) return dit->second;

        auto& out = _teamDetailCache[team];
        auto it = _teams.find(team);
        if (it != _teams.end()) {
            for (uint32_t pid : it->second) {
                int lvl = 1;
                auto lit = _playerLevel.find(pid);
                if (lit != _playerLevel.end()) lvl = lit->second;
                out.push_back({pid, lvl});
            }
            std::sort(out.begin(), out.end(), [](const PlayerDetailStat& a, const PlayerDetailStat& b) {
                return a.level > b.level;
            });
        }
        return out;
    }

private:
    void recomputeMax(const std::string&)
    {
        _rankDirty   = true;
        _detailDirty = true;
        ++_storeVersion;
    }

    std::unordered_map<std::string, std::unordered_set<uint32_t>> _teams;
    std::unordered_map<uint32_t, std::string>                      _playerTeam;
    std::unordered_map<uint32_t, int>                              _playerLevel;

    mutable std::vector<TeamStat>                                              _rankedCache;
    mutable std::unordered_map<std::string, std::vector<PlayerDetailStat>>    _teamDetailCache;
    mutable bool     _rankDirty    = true;
    mutable bool     _detailDirty  = true;
    uint64_t         _storeVersion = 1;
};

} // namespace zappy
