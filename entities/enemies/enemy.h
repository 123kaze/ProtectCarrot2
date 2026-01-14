#ifndef ENEMY_H
#define ENEMY_H

#include <QList>
#include <QPointF>
#include <QString>
#include <cstdint>

#include "../types.h"

class Enemy
{
public:
    Enemy(int id, EnemyType type, const QList<QPointF>& path);

    int id() const { return id_; }
    EnemyType type() const { return type_; }

    void update(std::int64_t deltaMs);

    void takeDamage(int damage);
    bool isDead() const { return hp_ <= 0; }

    int hp() const { return hp_; }

    QPointF position() const { return pos_; }
    QPointF renderPosition() const { return QPointF(pos_.x(), pos_.y() + bobbingOffsetY_); }
    QPointF centerPosition() const { return QPointF(pos_.x() + renderSize_ / 2.0, pos_.y() + renderSize_ / 2.0); }
    QString spriteName() const { return useAlt_ ? spriteAltName_ : spriteBaseName_; }

    bool isMovingLeft() const { return movedLastTick_ && lastMoveDx_ < -0.01; }

    bool isAtEnd() const { return atEnd_; }

    double progress() const { return traveledDistancePx_; }

private:
    void switchSprite();

    int id_ = -1;
    EnemyType type_ = EnemyType::Monster1;

    QList<QPointF> path_;
    int pathIndex_ = 0;

    QPointF pos_;

    double speedPxPerSec_ = 240.0;
    int hp_ = 0;

    QString spriteBaseName_;
    QString spriteAltName_;
    bool useAlt_ = false;

    bool atEnd_ = false;
    double traveledDistancePx_ = 0.0;

    int renderSize_ = 80;

    std::int64_t animAccMs_ = 0;

    bool movedLastTick_ = false;
    double lastMoveDx_ = 0.0;

    double animPhase_ = 0.0;
    double bobbingOffsetY_ = 0.0;
};

#endif
