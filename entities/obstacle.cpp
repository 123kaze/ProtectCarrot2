#include "obstacle.h"

#include <QRectF>

Obstacle::Obstacle(int id, const QPointF& pos, const QString& spriteName, int hp, int width, int height)
    : id_(id), pos_(pos), spriteName_(spriteName), hp_(hp), renderW_(width), renderH_(height)
{
}

void Obstacle::takeDamage(int damage)
{
    // AI辅助痕迹：此处参考了 AI 对“核心接口防御非法输入”的建议，
    // 我增加了 damage<=0 直接忽略的保护，避免异常输入导致血量反向变化。
    if (damage <= 0)
    {
        return;
    }

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
