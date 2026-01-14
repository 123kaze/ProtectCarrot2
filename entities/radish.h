#ifndef RADISH_H
#define RADISH_H

#include "../../core/gameobject.h"

class Radish : public GameObject {
public:
    Radish(QPointF pos) : GameObject(pos, "radish_10") {
        hp = 10;
    }

    void takeDamage(int damage) {
        // AI辅助痕迹：此处参考了 AI 对“受击后按 hp 切换贴图”的实现建议，
        // 我增加了 damage<=0 的输入保护，并把贴图更新与 hp 同步放在同一个接口里，确保状态一致。
        if (damage <= 0)
        {
            return;
        }
        hp -= damage;
        if (hp < 0) hp = 0;
        // 动态更新图片名,例如 hp 为 9 时，对应的图片是 radish_9.png
        if (hp > 0) {
            spriteName = QString("radish_%1").arg(hp);
        } else {
            spriteName = "lose";
        }
    }

    void update() override {
    }

    void draw(QPainter* painter) override {
        if (!painter)
        {
            return;
        }

        const int yOffset = -10;

        if (hp <= 0)
        {
            const QPixmap& losePix = ResourceManager::instance().getPixmap("lose");
            painter->drawPixmap(static_cast<int>(pos.x()), static_cast<int>(pos.y()) + yOffset,
                                losePix.scaled(100, 100, Qt::KeepAspectRatio));
            return;
        }

        const QPixmap& radishPix = ResourceManager::instance().getPixmap(spriteName);
        const QPixmap scaledRadish = radishPix.scaled(100, 100, Qt::KeepAspectRatio);
        const int radishX = static_cast<int>(pos.x());
        const int radishY = static_cast<int>(pos.y()) + yOffset;
        painter->drawPixmap(radishX, radishY, scaledRadish);

        const QString healthSprite = QString("health_%1").arg(hp);
        const QPixmap& healthPix = ResourceManager::instance().getPixmap(healthSprite);
        const QPixmap scaledHealth = healthPix.scaled(60, 38, Qt::KeepAspectRatio);

        // Align health bottom to radish bottom, slightly shift left, draw on top.
        const int healthX = radishX + scaledRadish.width() - 10;
        const int healthY = radishY + scaledRadish.height() - scaledHealth.height();
        painter->drawPixmap(healthX, healthY, scaledHealth);
    }

    int getHp() const { return hp; }

private:
    int hp;
};

#endif