#ifndef TOWERBASE_H
#define TOWERBASE_H

#include "../../core/gameobject.h"

class TowerBase : public GameObject {
public:
    TowerBase(QPointF pos, QString spriteName) : GameObject(pos, spriteName) {}

    void update() override {}
};

#endif
