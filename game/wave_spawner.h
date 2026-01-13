#ifndef WAVE_SPAWNER_H
#define WAVE_SPAWNER_H

#include <cstdint>
 #include <vector>

class GameWorld;

enum class EnemyType;

struct SpawnRule
{
    EnemyType type;
    std::int64_t intervalMs;
    std::int64_t nextSpawnMs;
};

class WaveSpawner
{
public:
    WaveSpawner();

    void reset();
    void update(GameWorld& world, std::int64_t deltaMs);

    int currentWave() const { return currentWave_; }
    int totalWaves() const { return totalWaves_; }

private:
    void spawn(GameWorld& world, EnemyType type);

    void setupWaveRules();
    void updateWaveRules(GameWorld& world, std::int64_t absoluteMs);

    int currentWave_ = 0;
    int totalWaves_ = 5;
    std::int64_t elapsedMs_ = 0;

    std::vector<SpawnRule> rules_;
};

#endif
