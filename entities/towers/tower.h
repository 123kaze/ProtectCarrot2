#ifndef TOWER_H
#define TOWER_H

#include <QPointF>
#include <QString>
#include <cstdint>
#include <memory>
#include <vector>

#include "../types.h"

class TargetingStrategy;

class Enemy;
class Obstacle;
class Bullet;

class Tower
{
public:
    Tower(int id, TowerType type, const QPointF& pos);
    ~Tower();

    int id() const { return id_; }
    TowerType type() const { return type_; }

    QPointF position() const { return pos_; }

    const QString& spriteName() const { return spriteName_; }

    int rangePx() const { return rangePx_; }

    int upgradeCost() const { return upgradeCost_; }
    int sellPrice() const { return sellPrice_; }

    int level() const { return level_; }

    double rotationDeg() const { return rotationDeg_; }

    bool contains(const QPointF& p) const;

    bool isInRange(const QPointF& target) const;

    bool update(std::int64_t deltaMs,
                const std::vector<std::unique_ptr<Enemy>>& enemies,
                Obstacle* selectedObstacle,
                std::vector<std::unique_ptr<Bullet>>& outBullets);

    bool upgrade();

private:
    Enemy* chooseTargetEnemy(const std::vector<std::unique_ptr<Enemy>>& enemies) const;

    QString bulletSprite() const;
    std::int64_t fireIntervalMs() const;

    int id_ = -1;
    TowerType type_ = TowerType::Cannon;

    QPointF pos_;

    int level_ = 1;

    int rangePx_ = 200;
    int damage_ = 50;

    int upgradeCost_ = 180;
    int sellPrice_ = 50;

    QString spriteName_;
    QString upgradedSpriteName_;

    std::int64_t timeSinceShotMs_ = 0;

    std::unique_ptr<TargetingStrategy> targeting_;

    double rotationDeg_ = 0.0;
};

#endif
