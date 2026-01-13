#include "obstacle.h"

#include <QRectF>

Obstacle::Obstacle(int id, const QPointF& pos, const QString& spriteName, int hp, int width, int height)
    : id_(id), pos_(pos), spriteName_(spriteName), hp_(hp), renderW_(width), renderH_(height)
{
}

void Obstacle::takeDamage(int damage)
{
    hp_ -= damage;
    if (hp_ < 0)
    {
        hp_ = 0;
    }
}

bool Obstacle::contains(const QPointF& p) const
{
    const QRectF r(pos_.x(), pos_.y(), renderW_, renderH_);
    return r.contains(p);
}
