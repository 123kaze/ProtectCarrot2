#include "wave_spawner.h"

#include "game_world.h"
#include "../entities/types.h"

#include <limits>

WaveSpawner::WaveSpawner() { reset(); }

void WaveSpawner::reset()
{
    currentWave_ = 0;
    totalWaves_ = 5;
    elapsedMs_ = 0;

    setupWaveRules();
}

void WaveSpawner::spawn(GameWorld& world, EnemyType type)
{
    world.spawnEnemy(type);
}

void WaveSpawner::setupWaveRules()
{
    rules_.clear();

    // WendyAr timings are based on absolute time (ms) since game start.
    // We keep that behavior but trigger spawns using "nextSpawnMs" accumulation.
    const std::int64_t base = (currentWave_ == 0) ? 0 : (currentWave_ == 1) ? 20000
                                     : (currentWave_ == 2) ? 40000
                                     : (currentWave_ == 3) ? 60000
                                     : 80000;

    if (currentWave_ == 0)
    {
        rules_.push_back({EnemyType::Monster1, 3000, base + 3000});
        rules_.push_back({EnemyType::Monster2, 5000, base + 5000});
        return;
    }

    if (currentWave_ == 1)
    {
        rules_.push_back({EnemyType::Monster3, 3000, base + 3000});
        rules_.push_back({EnemyType::Monster2, 5000, base + 5000});
        return;
    }

    if (currentWave_ == 2)
    {
        rules_.push_back({EnemyType::Monster4, 3000, base + 3000});
        rules_.push_back({EnemyType::Monster5, 5000, base + 5000});
        return;
    }

    if (currentWave_ == 3)
    {
        rules_.push_back({EnemyType::Monster1, 3000, base + 3000});
        rules_.push_back({EnemyType::Monster5, 5000, base + 5000});
        return;
    }

    // wave 4
    rules_.push_back({EnemyType::Monster1, 2000, base + 2000});
    rules_.push_back({EnemyType::Monster2, 4000, base + 4000});
    rules_.push_back({EnemyType::Monster3, 5000, base + 5000});
    rules_.push_back({EnemyType::MonsterBoss, 0, 110000});
}

void WaveSpawner::updateWaveRules(GameWorld& world, std::int64_t absoluteMs)
{
    for (auto& r : rules_)
    {
        if (r.intervalMs <= 0)
        {
            if (absoluteMs >= r.nextSpawnMs)
            {
                spawn(world, r.type);
                r.nextSpawnMs = std::numeric_limits<std::int64_t>::max();
            }
            continue;
        }

        while (absoluteMs >= r.nextSpawnMs)
        {
            spawn(world, r.type);
            r.nextSpawnMs += r.intervalMs;
        }
    }
}

void WaveSpawner::update(GameWorld& world, std::int64_t deltaMs)
{
    elapsedMs_ += deltaMs;

    // Advance waves based on absolute time.
    const int newWave = (elapsedMs_ < 20000) ? 0 : (elapsedMs_ < 40000) ? 1 : (elapsedMs_ < 60000) ? 2
                                 : (elapsedMs_ < 80000) ? 3
                                 : 4;

    if (newWave != currentWave_)
    {
        currentWave_ = newWave;
        setupWaveRules();
    }

    updateWaveRules(world, elapsedMs_);
}
