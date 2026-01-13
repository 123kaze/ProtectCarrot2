#include "enemy.h"

#include <QtMath>

Enemy::Enemy(int id, EnemyType type, const QList<QPointF>& path) : id_(id), type_(type), path_(path)
{
    if (path_.isEmpty())
    {
        path_.append(QPointF());
    }

    pos_ = path_.front();
    pathIndex_ = 0;

    switch (type_)
    {
        case EnemyType::Monster1:
            speedPxPerSec_ = 40.0;
            hp_ = 200;
            spriteBaseName_ = "monster1";
            spriteAltName_ = "monster1_2";
            renderSize_ = 80;
            break;
        case EnemyType::Monster2:
            speedPxPerSec_ = 60.0;
            hp_ = 200;
            spriteBaseName_ = "monster2";
            spriteAltName_ = "monster2_2";
            renderSize_ = 80;
            break;
        case EnemyType::Monster3:
            speedPxPerSec_ = 40.0;
            hp_ = 250;
            spriteBaseName_ = "monster3";
            spriteAltName_ = "monster3_2";
            renderSize_ = 80;
            break;
        case EnemyType::Monster4:
            speedPxPerSec_ = 35.0;
            hp_ = 250;
            spriteBaseName_ = "monster4";
            spriteAltName_ = "monster4_2";
            renderSize_ = 80;
            break;
        case EnemyType::Monster5:
            speedPxPerSec_ = 40.0;
            hp_ = 250;
            spriteBaseName_ = "monster5";
            spriteAltName_ = "monster5_2";
            renderSize_ = 80;
            break;
        case EnemyType::MonsterBoss:
            speedPxPerSec_ = 20.0;
            hp_ = 3000;
            spriteBaseName_ = "monsterboss";
            spriteAltName_ = "monsterboss";
            renderSize_ = 120;
            break;
    }
}

void Enemy::takeDamage(int damage)
{
    if (hp_ <= 0)
    {
        return;
    }

    hp_ -= damage;
    if (hp_ < 0)
    {
        hp_ = 0;
    }
}

void Enemy::switchSprite()
{
    useAlt_ = !useAlt_;
}

void Enemy::update(std::int64_t deltaMs)
{
    if (hp_ <= 0 || atEnd_)
    {
        return;
    }

    const QPointF oldPos = pos_;

    if (pathIndex_ >= path_.size())
    {
        atEnd_ = true;
        return;
    }

    const QPointF target = path_[pathIndex_];

    const QPointF diff = target - pos_;
    const double dist = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());

    const double step = speedPxPerSec_ * (static_cast<double>(deltaMs) / 1000.0);

    if (dist <= step)
    {
        traveledDistancePx_ += dist;
        pos_ = target;
        pathIndex_ += 1;
        if (pathIndex_ >= path_.size())
        {
            atEnd_ = true;
        }
        // fallthrough to animation update
    }

    else if (dist > 0.0)
    {
        const double nx = diff.x() / dist;
        const double ny = diff.y() / dist;
        pos_.setX(pos_.x() + nx * step);
        pos_.setY(pos_.y() + ny * step);
        traveledDistancePx_ += step;
    }

    const bool moved = (qAbs(pos_.x() - oldPos.x()) + qAbs(pos_.y() - oldPos.y())) > 0.01;
    if (moved)
    {
        animAccMs_ += deltaMs;
        if (animAccMs_ >= 100)
        {
            animAccMs_ = 0;
            switchSprite();
        }

        // 轻微上下浮动：移动时更有“走路”感觉
        animPhase_ += static_cast<double>(deltaMs) / 1000.0;
        bobbingOffsetY_ = qSin(animPhase_ * 12.0) * 3.0;
    }
    else
    {
        // 停止时回到静止
        animAccMs_ = 0;
        bobbingOffsetY_ *= 0.8;
        if (qAbs(bobbingOffsetY_) < 0.05)
        {
            bobbingOffsetY_ = 0.0;
        }
    }
}
