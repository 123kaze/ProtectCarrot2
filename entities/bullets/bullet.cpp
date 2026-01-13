#include "bullet.h"

#include <QtMath>

#include "../enemies/enemy.h"
#include "../obstacle.h"

Bullet::Bullet(const QPointF& startPos, Enemy* target, int damage, const QString& spriteName)
    : pos_(startPos), enemyTarget_(target), damage_(damage), spriteName_(spriteName)
{
    shouldRotate_ = spriteName_.contains("star_bullet") || spriteName_.contains("fan_bullet");
}

Bullet::Bullet(const QPointF& startPos, Obstacle* target, int damage, const QString& spriteName)
    : pos_(startPos), obstacleTarget_(target), damage_(damage), spriteName_(spriteName)
{
    shouldRotate_ = spriteName_.contains("star_bullet") || spriteName_.contains("fan_bullet");
}

void Bullet::update(std::int64_t deltaMs)
{
    if (expired_)
    {
        return;
    }

    if (enemyTarget_ && enemyTarget_->isDead())
    {
        expired_ = true;
        return;
    }

    if (obstacleTarget_ && obstacleTarget_->isDead())
    {
        expired_ = true;
        return;
    }

    const QPointF tgt = targetPosition();
    const QPointF diff = tgt - pos_;
    const double dist = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());

    const double step = speedPxPerSec_ * (static_cast<double>(deltaMs) / 1000.0);
    const double hitThreshold = step;

    if (dist <= 1.0)
    {
        tryHitTarget(hitThreshold);
        return;
    }

    if (dist <= step)
    {
        pos_ = tgt;
        tryHitTarget(hitThreshold);
        return;
    }

    const double nx = diff.x() / dist;
    const double ny = diff.y() / dist;
    pos_.setX(pos_.x() + nx * step);
    pos_.setY(pos_.y() + ny * step);

    if (shouldRotate_)
    {
        // 对齐 /.WendyAr：每 100ms 旋转约 60 度
        rotationDeg_ += 600.0 * (static_cast<double>(deltaMs) / 1000.0);
    }

    tryHitTarget(hitThreshold);
}

QPointF Bullet::targetPosition() const
{
    if (enemyTarget_)
    {
        return enemyTarget_->centerPosition();
    }

    if (obstacleTarget_)
    {
        return obstacleTarget_->centerPosition();
    }

    return QPointF();
}

void Bullet::tryHitTarget(double hitThresholdPx)
{
    const QPointF tgt = targetPosition();
    if (tgt.isNull())
    {
        expired_ = true;
        return;
    }

    const QPointF diff = tgt - pos_;
    const double manhattan = qAbs(diff.x()) + qAbs(diff.y());

    if (manhattan > hitThresholdPx)
    {
        return;
    }

    if (enemyTarget_)
    {
        enemyTarget_->takeDamage(damage_);
    }
    else if (obstacleTarget_)
    {
        obstacleTarget_->takeDamage(damage_);
    }

    expired_ = true;
}
