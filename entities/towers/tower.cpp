#include "tower.h"

#include <QRectF>
#include <QtMath>

#include "../bullets/bullet.h"
#include "../enemies/enemy.h"
#include "../obstacle.h"
#include "targeting_strategy.h"

Tower::Tower(int id, TowerType type, const QPointF& pos) : id_(id), type_(type), pos_(pos)
{
    targeting_ = std::make_unique<FurthestProgressStrategy>();

    switch (type_)
    {
        case TowerType::Cannon:
            spriteName_ = "cannon";
            upgradedSpriteName_ = "cannon_upgraded";
            rangePx_ = 200;
            damage_ = 50;
            upgradeCost_ = 180;
            sellPrice_ = 50;
            break;
        case TowerType::Poop:
            spriteName_ = "poop";
            upgradedSpriteName_ = "poop_upgraded";
            rangePx_ = 200;
            damage_ = 50;
            upgradeCost_ = 180;
            sellPrice_ = 50;
            break;
        case TowerType::Star:
            spriteName_ = "star";
            upgradedSpriteName_ = "star_upgraded";
            rangePx_ = 240;
            damage_ = 80;
            upgradeCost_ = 180;
            sellPrice_ = 50;
            break;
        case TowerType::Fan:
            spriteName_ = "fan";
            upgradedSpriteName_ = "fan_upgraded";
            rangePx_ = 240;
            damage_ = 80;
            upgradeCost_ = 180;
            sellPrice_ = 50;
            break;
    }
}

Tower::~Tower() = default;

bool Tower::contains(const QPointF& p) const
{
    const QRectF r(pos_.x() - 40, pos_.y() - 40, 80, 80);
    return r.contains(p);
}

bool Tower::isInRange(const QPointF& target) const
{
    const qreal dx = target.x() - pos_.x();
    const qreal dy = target.y() - pos_.y();
    const qreal dist = qSqrt(dx * dx + dy * dy);
    return dist <= rangePx_;
}

std::int64_t Tower::fireIntervalMs() const
{
    switch (type_)
    {
        case TowerType::Cannon:
            return 500;
        case TowerType::Poop:
            return 1000;
        case TowerType::Star:
            return 1500;
        case TowerType::Fan:
            return 2000;
    }

    return 1000;
}

QString Tower::bulletSprite() const
{
    switch (type_)
    {
        case TowerType::Cannon:
            return "cannon_bullet";
        case TowerType::Poop:
            return "poop_bullet";
        case TowerType::Star:
            return "star_bullet";
        case TowerType::Fan:
            return "fan_bullet";
    }

    return "cannon_bullet";
}

Enemy* Tower::chooseTargetEnemy(const std::vector<std::unique_ptr<Enemy>>& enemies) const
{
    if (!targeting_)
    {
        return nullptr;
    }

    std::vector<Enemy*> candidates;
    candidates.reserve(enemies.size());

    for (const auto& e : enemies)
    {
        if (!e || e->isDead())
        {
            continue;
        }
        if (!isInRange(e->centerPosition()))
        {
            continue;
        }
        candidates.push_back(e.get());
    }

    return targeting_->chooseTarget(candidates);
}

void Tower::update(std::int64_t deltaMs,
                   const std::vector<std::unique_ptr<Enemy>>& enemies,
                   Obstacle* selectedObstacle,
                   std::vector<std::unique_ptr<Bullet>>& outBullets)
{
    // AI辅助痕迹：这里参考了旧工程 /.WendyAr 的思路：先选目标（优先障碍物，其次敌人），
    // 然后根据目标位置计算炮塔旋转角度。为适配当前架构，我将“选目标”逻辑保留在 Tower 内，
    // 但目标排序可通过 TargetingStrategy 扩展。
    QPointF targetPos;
    if (selectedObstacle && !selectedObstacle->isDead() && isInRange(selectedObstacle->centerPosition()))
    {
        targetPos = selectedObstacle->centerPosition();
    }
    else
    {
        Enemy* preview = chooseTargetEnemy(enemies);
        if (preview)
        {
            targetPos = preview->centerPosition();
        }
    }

    if (!targetPos.isNull())
    {
        const QPointF diff = targetPos - pos_;
        rotationDeg_ = qAtan2(diff.y(), diff.x()) * 180.0 / M_PI + 90.0;
    }

    timeSinceShotMs_ += deltaMs;

    const std::int64_t interval = fireIntervalMs();
    if (timeSinceShotMs_ < interval)
    {
        return;
    }

    if (selectedObstacle && !selectedObstacle->isDead() && isInRange(selectedObstacle->centerPosition()))
    {
        outBullets.push_back(std::make_unique<Bullet>(pos_.toPoint(), selectedObstacle, damage_, bulletSprite()));
        timeSinceShotMs_ = 0;
        return;
    }

    Enemy* target = chooseTargetEnemy(enemies);
    if (!target)
    {
        return;
    }

    outBullets.push_back(std::make_unique<Bullet>(pos_.toPoint(), target, damage_, bulletSprite()));
    timeSinceShotMs_ = 0;
}

bool Tower::upgrade()
{
    level_ += 1;
    damage_ += 20;
    rangePx_ += 30;
    upgradeCost_ += 30;

    spriteName_ = upgradedSpriteName_;

    return true;
}
