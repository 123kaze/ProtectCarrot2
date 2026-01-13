#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <QPointF>
#include <QRectF>
#include <QList>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "game_state.h"

class Enemy;
class Tower;
class Bullet;
class Obstacle;
class TowerPit;
class Radish;

enum class TowerType;
enum class EnemyType;

struct SelectedObject
{
    enum class Kind
    {
        None,
        Tower,
        Obstacle
    };

    Kind kind = Kind::None;
    int id = -1;
};

class GameWorld
{
public:
    GameWorld();
    ~GameWorld();

    void reset();
    void update(std::int64_t deltaMs);

    bool togglePause();
    void toggleSpeed();

    int money() const { return money_; }
    bool isPaused() const { return state_ == GameState::Paused; }
    int speedMultiplier() const { return speedMultiplier_; }

    GameState state() const { return state_; }

    bool consumeEnemyDeathSfxRequested();
    bool consumeVictorySfxRequested();

    std::int64_t countdownMsLeft() const { return countdownMsLeft_; }

    const std::vector<std::unique_ptr<Enemy>>& enemies() const { return enemies_; }
    const std::vector<std::unique_ptr<Tower>>& towers() const { return towers_; }
    const std::vector<std::unique_ptr<Obstacle>>& obstacles() const { return obstacles_; }
    const std::vector<std::unique_ptr<TowerPit>>& pits() const { return pits_; }
    Radish& radish();
    const Radish& radish() const;

    const std::vector<std::unique_ptr<Bullet>>& bullets() const { return bullets_; }

    void startCountdown();

    bool handlePitClick(const QPointF& scenePos);
    bool handleTowerOrObstacleClick(const QPointF& scenePos);

    bool buildTowerAtSelectedPit(TowerType type);

    bool upgradeSelectedTower();
    bool sellSelectedTower();

    SelectedObject selected() const { return selected_; }

    int waveIndex() const;
    int totalWaves() const;

    void spawnEnemy(EnemyType type);

private:
    void initLevel();

    int findPitIndexAt(const QPointF& scenePos) const;
    int findTowerIndexAt(const QPointF& scenePos) const;
    int findObstacleIndexAt(const QPointF& scenePos) const;

    void updateEnemies(std::int64_t deltaMs);
    void updateTowersAndBullets(std::int64_t deltaMs);
    void resolveBullets(std::int64_t deltaMs);
    void resolveDeathsAndRewards();
    void checkRadishCollision();
    void updateState();

    bool canAfford(int cost) const;

    GameState state_ = GameState::Start;
    int speedMultiplier_ = 1;

    int money_ = 450;

    QList<QPointF> path_;

    std::vector<std::unique_ptr<Enemy>> enemies_;
    std::vector<std::unique_ptr<TowerPit>> pits_;
    std::vector<std::unique_ptr<Tower>> towers_;
    std::vector<std::unique_ptr<Obstacle>> obstacles_;
    std::vector<std::unique_ptr<Bullet>> bullets_;

    std::unique_ptr<Radish> radish_;

    int selectedPitIndex_ = -1;
    SelectedObject selected_;

    std::int64_t moneyAccMs_ = 0;

    std::int64_t countdownMsLeft_ = 0;

    std::unique_ptr<class WaveSpawner> waveSpawner_;

    bool enemyDeathSfxRequested_ = false;
    bool victorySfxRequested_ = false;
};

#endif
