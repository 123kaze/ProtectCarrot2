#ifndef RADISH_H
#define RADISH_H

#include "../../core/gameobject.h"

class Radish : public GameObject {
public:
    Radish(QPointF pos) : GameObject(pos, "radish_10") {
        hp = 10;
    }

    void takeDamage(int damage) {
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

    int getHp() const { return hp; }

private:
    int hp;
};

#endif