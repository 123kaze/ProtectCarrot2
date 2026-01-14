#include "game_world.h"

#include <algorithm>

#include "wave_spawner.h"

#include "../entities/bullets/bullet.h"
#include "../entities/enemies/enemy.h"
#include "../entities/obstacle.h"
#include "../entities/radish.h"
#include "../entities/towers/tower.h"
#include "../entities/towers/towerpit.h"
#include "../entities/types.h"

namespace {

struct TowerTypeInfo
{
    int cost = 0;
    const char* buttonSprite = nullptr;
    TowerType type = TowerType::Cannon;
};

int towerCost(TowerType type)
{
    switch (type)
    {
        case TowerType::Cannon:
            return 100;
        case TowerType::Poop:
            return 120;
        case TowerType::Star:
            return 160;
        case TowerType::Fan:
            return 160;
    }

    return 0;
}

int killReward(EnemyType type)
{
    switch (type)
    {
        case EnemyType::Monster1:
            return 50;
        case EnemyType::Monster2:
        case EnemyType::Monster3:
        case EnemyType::Monster4:
        case EnemyType::Monster5:
            return 80;
        case EnemyType::MonsterBoss:
            return 200;
    }

    return 0;
}

int radishDamage(EnemyType type)
{
    switch (type)
    {
        case EnemyType::Monster1:
            return 1;
        case EnemyType::Monster2:
        case EnemyType::Monster3:
        case EnemyType::Monster4:
        case EnemyType::Monster5:
            return 1;
        case EnemyType::MonsterBoss:
            return 10;
    }

    return 1;
}

} // namespace

GameWorld::GameWorld() { reset(); }

GameWorld::~GameWorld() = default;

bool GameWorld::consumeEnemyDeathSfxRequested()
{
    const bool v = enemyDeathSfxRequested_;
    enemyDeathSfxRequested_ = false;
    return v;
}

bool GameWorld::consumeVictorySfxRequested()
{
    const bool v = victorySfxRequested_;
    victorySfxRequested_ = false;
    return v;
}

std::vector<TowerType> GameWorld::consumeTowerFireSfxEvents()
{
    std::vector<TowerType> out;
    out.swap(towerFireSfxEvents_);
    return out;
}

void GameWorld::reset()
{
    state_ = GameState::Start;
    speedMultiplier_ = 1;

    enemyDeathSfxRequested_ = false;
    victorySfxRequested_ = false;
    towerFireSfxEvents_.clear();

    money_ = 450;
    moneyAccMs_ = 0;

    enemies_.clear();
    pits_.clear();
    towers_.clear();
    obstacles_.clear();
    bullets_.clear();

    selectedPitIndex_ = -1;
    selected_ = {};

    countdownMsLeft_ = 0;

    waveSpawner_ = std::make_unique<WaveSpawner>();
    waveSpawner_->reset();

    initLevel();
}

const Radish& GameWorld::radish() const
{
    return *radish_;
}

Radish& GameWorld::radish()
{
    return *radish_;
}

void GameWorld::initLevel()
{
    // 1200x800 map coordinates
    path_.clear();
    path_ << QPointF(400, 200) << QPointF(930, 200) << QPointF(930, 400) << QPointF(210, 400)
          << QPointF(210, 600) << QPointF(700, 600);

    radish_ = std::make_unique<Radish>(QPointF(750, 600));

    pits_ = TowerPit::createTowerPits();

    // Place obstacles on specific pits (same indices as reference)
    const std::vector<int> targetIndices = {9, 15, 17, 19, 20, 22, 25, 33, 34, 38};
    const std::vector<QString> sprites = {"obstacle1", "obstacle2", "obstacle3", "obstacle4", "obstacle5",
                                          "obstacle6", "obstacle7", "obstacle8", "obstacle9", "obstacle10"};
    const std::vector<QSize> sizes = {QSize(180, 180), QSize(100, 100), QSize(100, 100), QSize(100, 100),
                                      QSize(100, 100), QSize(100, 100), QSize(180, 180), QSize(180, 180),
                                      QSize(100, 100), QSize(180, 180)};

    int idx = 0;
    int obstacleId = 0;
    for (const int pitIndex : targetIndices)
    {
        if (idx >= static_cast<int>(sprites.size()))
        {
            break;
        }

        for (auto& pit : pits_)
        {
            if (pit && pit->index() == pitIndex)
            {
                const QSize sz = sizes[idx];
                obstacles_.push_back(
                    std::make_unique<Obstacle>(obstacleId++, pit->topLeft(), sprites[idx], 300, sz.width(), sz.height()));
                pit->setHasObstacle(true);
                idx++;
                break;
            }
        }
    }
}

bool GameWorld::togglePause()
{
    if (state_ == GameState::Running)
    {
        state_ = GameState::Paused;
        return true;
    }
    if (state_ == GameState::Paused)
    {
        state_ = GameState::Running;
        return true;
    }
    return false;
}

void GameWorld::toggleSpeed()
{
    speedMultiplier_ = (speedMultiplier_ == 1) ? 2 : 1;
}

void GameWorld::startCountdown()
{
    if (state_ != GameState::Start)
    {
        return;
    }

    state_ = GameState::Countdown;
    countdownMsLeft_ = 4000;
}

int GameWorld::waveIndex() const
{
    return waveSpawner_ ? waveSpawner_->currentWave() : 0;
}

int GameWorld::totalWaves() const
{
    return waveSpawner_ ? waveSpawner_->totalWaves() : 5;
}

void GameWorld::spawnEnemy(EnemyType type)
{
    const int id = static_cast<int>(enemies_.size()) + 1;
    enemies_.push_back(std::make_unique<Enemy>(id, type, path_));
}

bool GameWorld::canAfford(int cost) const
{
    return money_ >= cost;
}

int GameWorld::findPitIndexAt(const QPointF& scenePos) const
{
    for (int i = 0; i < static_cast<int>(pits_.size()); ++i)
    {
        const auto& pit = pits_[i];
        if (pit && pit->contains(scenePos))
        {
            return i;
        }
    }

    return -1;
}

int GameWorld::findTowerIndexAt(const QPointF& scenePos) const
{
    for (int i = 0; i < static_cast<int>(towers_.size()); ++i)
    {
        const auto& t = towers_[i];
        if (t && t->contains(scenePos))
        {
            return i;
        }
    }
    return -1;
}

int GameWorld::findObstacleIndexAt(const QPointF& scenePos) const
{
    for (int i = 0; i < static_cast<int>(obstacles_.size()); ++i)
    {
        const auto& o = obstacles_[i];
        if (o && o->contains(scenePos))
        {
            return i;
        }
    }
    return -1;
}

bool GameWorld::handlePitClick(const QPointF& scenePos)
{
    const int pitIdx = findPitIndexAt(scenePos);
    if (pitIdx < 0)
    {
        return false;
    }

    auto& pit = pits_[pitIdx];
    if (!pit || pit->hasTower() || pit->hasObstacle())
    {
        return false;
    }

    selectedPitIndex_ = pitIdx;
    selected_ = {};

    return true;
}

bool GameWorld::handleTowerOrObstacleClick(const QPointF& scenePos)
{
    const int obstacleIdx = findObstacleIndexAt(scenePos);
    if (obstacleIdx >= 0)
    {
        for (auto& o : obstacles_)
        {
            if (o)
            {
                o->setSelected(false);
            }
        }

        const int id = obstacles_[obstacleIdx]->id();
        obstacles_[obstacleIdx]->setSelected(true);
        selected_ = {SelectedObject::Kind::Obstacle, id};
        selectedPitIndex_ = -1;
        return true;
    }

    const int towerIdx = findTowerIndexAt(scenePos);
    if (towerIdx >= 0)
    {
        const int id = towers_[towerIdx]->id();
        if (selected_.kind == SelectedObject::Kind::Tower && selected_.id == id)
        {
            selected_ = {};
        }
        else
        {
            selected_ = {SelectedObject::Kind::Tower, id};
        }
        selectedPitIndex_ = -1;
        return true;
    }

    return false;
}

bool GameWorld::buildTowerAtSelectedPit(TowerType type)
{
    if (selectedPitIndex_ < 0 || selectedPitIndex_ >= static_cast<int>(pits_.size()))
    {
        return false;
    }

    auto& pit = pits_[selectedPitIndex_];
    if (!pit || pit->hasTower() || pit->hasObstacle())
    {
        return false;
    }

    const int cost = towerCost(type);
    if (!canAfford(cost))
    {
        selectedPitIndex_ = -1;
        return false;
    }

    const int id = static_cast<int>(towers_.size()) + 1;
    towers_.push_back(std::make_unique<Tower>(id, type, pit->center()));
    pit->setHasTower(true);

    money_ -= cost;

    selectedPitIndex_ = -1;
    return true;
}

bool GameWorld::upgradeSelectedTower()
{
    if (selected_.kind != SelectedObject::Kind::Tower)
    {
        return false;
    }

    for (auto& t : towers_)
    {
        if (t && t->id() == selected_.id)
        {
            if (t->level() >= 2)
            {
                return false;
            }

            const int cost = t->upgradeCost();
            if (!canAfford(cost))
            {
                return false;
            }

            if (t->upgrade())
            {
                money_ -= cost;
                return true;
            }
        }
    }

    return false;
}

bool GameWorld::sellSelectedTower()
{
    if (selected_.kind != SelectedObject::Kind::Tower)
    {
        return false;
    }

    for (std::size_t i = 0; i < towers_.size(); ++i)
    {
        auto& t = towers_[i];
        if (t && t->id() == selected_.id)
        {
            money_ += t->sellPrice();

            // free the pit
            const QPointF pos = t->position();
            for (auto& pit : pits_)
            {
                if (pit && pit->center() == pos)
                {
                    pit->setHasTower(false);
                    break;
                }
            }

            towers_.erase(towers_.begin() + static_cast<long>(i));
            selected_ = {};
            return true;
        }
    }

    return false;
}

void GameWorld::update(std::int64_t deltaMs)
{
    if (state_ == GameState::Start)
    {
        return;
    }

    if (state_ == GameState::Countdown)
    {
        countdownMsLeft_ -= deltaMs;
        if (countdownMsLeft_ <= 0)
        {
            countdownMsLeft_ = 0;
            state_ = GameState::Running;
        }
        return;
    }

    if (state_ == GameState::Paused || state_ == GameState::Victory || state_ == GameState::Defeat)
    {
        return;
    }

    const std::int64_t scaledMs = deltaMs * speedMultiplier_;

    if (waveSpawner_)
    {
        waveSpawner_->update(*this, scaledMs);
    }

    updateEnemies(scaledMs);
    updateTowersAndBullets(scaledMs);
    resolveBullets(scaledMs);

    resolveDeathsAndRewards();
    checkRadishCollision();
    updateState();

    moneyAccMs_ += scaledMs;
    while (moneyAccMs_ >= 1000)
    {
        money_ += 1;
        moneyAccMs_ -= 1000;
    }
}

void GameWorld::updateEnemies(std::int64_t deltaMs)
{
    for (auto& e : enemies_)
    {
        if (e)
        {
            e->update(deltaMs);
        }
    }
}

void GameWorld::updateTowersAndBullets(std::int64_t deltaMs)
{
    Obstacle* selectedObstacle = nullptr;
    if (selected_.kind == SelectedObject::Kind::Obstacle)
    {
        for (auto& o : obstacles_)
        {
            if (o && o->id() == selected_.id)
            {
                selectedObstacle = o.get();
                break;
            }
        }
    }

    for (auto& t : towers_)
    {
        if (t)
        {
            const bool fired = t->update(deltaMs, enemies_, selectedObstacle, bullets_);
            if (fired)
            {
                towerFireSfxEvents_.push_back(t->type());
            }
        }
    }

    for (auto& b : bullets_)
    {
        if (b)
        {
            b->update(deltaMs);
        }
    }
}

void GameWorld::resolveBullets(std::int64_t)
{
    bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
                                 [](const std::unique_ptr<Bullet>& b) { return !b || b->isExpired(); }),
                  bullets_.end());
}

void GameWorld::resolveDeathsAndRewards()
{
    for (auto it = enemies_.begin(); it != enemies_.end();)
    {
        Enemy* e = it->get();
        if (!e)
        {
            it = enemies_.erase(it);
            continue;
        }

        if (e->isDead())
        {
            // AI辅助痕迹：此处参考了 AI 对“容器 erase 会导致外部裸指针悬垂”的提醒，
            // 我选择在 GameWorld 内统一做目标清理（而不是让 Bullet 自己遍历 enemies_），从生命周期源头保证安全。
            // 敌人即将被销毁，必须先让仍追踪它的子弹失效，避免悬垂指针。
            for (auto& b : bullets_)
            {
                if (b && b->enemyTarget_ == e)
                {
                    b->expired_ = true;
                    b->enemyTarget_ = nullptr;
                }
            }

            money_ += killReward(e->type());
            enemyDeathSfxRequested_ = true;
            it = enemies_.erase(it);
            continue;
        }

        ++it;
    }

    for (auto it = obstacles_.begin(); it != obstacles_.end();)
    {
        Obstacle* o = it->get();
        if (!o)
        {
            it = obstacles_.erase(it);
            continue;
        }

        if (o->isDead())
        {
            // If an obstacle is removed, any bullet still tracking it must be expired first.
            // Otherwise bullets will hold a dangling pointer and may start 'flying randomly'.
            for (auto& b : bullets_)
            {
                if (b && b->obstacleTarget_ == o)
                {
                    b->expired_ = true;
                    b->obstacleTarget_ = nullptr;
                }
            }

            if (selected_.kind == SelectedObject::Kind::Obstacle && selected_.id == o->id())
            {
                selected_ = {};
            }
            it = obstacles_.erase(it);
            continue;
        }

        ++it;
    }
}

void GameWorld::checkRadishCollision()
{
    if (!radish_ || radish_->getHp() <= 0)
    {
        return;
    }

    for (const auto& e : enemies_)
    {
        if (!e)
        {
            continue;
        }

        if (e->isAtEnd())
        {
            radish_->takeDamage(radishDamage(e->type()));
        }
    }

    // AI辅助痕迹：此处参考了 AI 的“删除前先断开引用关系”的建议，
    // 我在敌人到达终点被批量 remove_if 前先统一失效相关子弹，避免随机崩溃。
    // 到达终点的敌人会被移除，先让追踪它们的子弹失效。
    for (const auto& e : enemies_)
    {
        if (!e || !e->isAtEnd())
        {
            continue;
        }
        for (auto& b : bullets_)
        {
            if (b && b->enemyTarget_ == e.get())
            {
                b->expired_ = true;
                b->enemyTarget_ = nullptr;
            }
        }
    }

    enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
                                 [](const std::unique_ptr<Enemy>& e) { return e && e->isAtEnd(); }),
                  enemies_.end());
}

void GameWorld::updateState()
{
    if (radish_ && radish_->getHp() <= 0)
    {
        state_ = GameState::Defeat;
        return;
    }

    // Victory if boss is killed and no more enemies alive after it spawned.
    // if elapsed time reached boss window and no boss exists and no enemies.
    if (waveSpawner_ && waveSpawner_->currentWave() == 4)
    {
        const bool anyBoss = std::any_of(enemies_.begin(), enemies_.end(),
                                        [](const std::unique_ptr<Enemy>& e) {
                                            return e && e->type() == EnemyType::MonsterBoss;
                                        });
        if (!anyBoss && enemies_.empty())
        {
            if (state_ != GameState::Victory)
            {
                victorySfxRequested_ = true;
            }
            state_ = GameState::Victory;
        }
    }
}
