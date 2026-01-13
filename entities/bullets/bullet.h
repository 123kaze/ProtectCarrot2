#ifndef BULLET_H
#define BULLET_H

#include <QPointF>
#include <QString>
#include <cstdint>

class Enemy;
class Obstacle;

class Bullet
{
public:
    Bullet(const QPointF& startPos, Enemy* target, int damage, const QString& spriteName);
    Bullet(const QPointF& startPos, Obstacle* target, int damage, const QString& spriteName);

    void update(std::int64_t deltaMs);

    bool isExpired() const { return expired_; }

    QPointF position() const { return pos_; }
    const QString& spriteName() const { return spriteName_; }

    bool shouldRotate() const { return shouldRotate_; }
    double rotationDeg() const { return rotationDeg_; }

private:
    QPointF targetPosition() const;
    void tryHitTarget(double hitThresholdPx);

    QPointF pos_;

    Enemy* enemyTarget_ = nullptr;
    Obstacle* obstacleTarget_ = nullptr;

    int damage_ = 0;
    QString spriteName_;

    bool expired_ = false;
    double speedPxPerSec_ = 200.0;

    bool shouldRotate_ = false;
    double rotationDeg_ = 0.0;

    friend class GameWorld;
};

#endif
