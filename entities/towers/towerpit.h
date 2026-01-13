#ifndef TOWERPIT_H
#define TOWERPIT_H

#include "../../core/gameobject.h"

#include <QRectF>
#include <QList>
#include <memory>
#include <vector>

class TowerPit : public GameObject {
public:
    TowerPit(QPointF pos, int index)
        : GameObject(pos, "tower_pit"), rect(pos.x() - 40, pos.y() - 40, 80, 80), index_(index)
    {
        hasTower_ = false;
        hasObstacle_ = false;
        renderWidth = 80;
    }

    void update() override {}

    bool contains(QPointF p) const { return rect.contains(p); }
    bool canPlace() const { return !hasTower_ && !hasObstacle_; }
    bool hasTower() const { return hasTower_; }
    void setHasTower(bool b) { hasTower_ = b; }
    QPointF center() const { return pos; }

    int index() const { return index_; }

    bool hasObstacle() const { return hasObstacle_; }
    void setHasObstacle(bool b) { hasObstacle_ = b; }

    QPointF position() const { return pos; }
    QPointF topLeft() const { return QPointF(pos.x() - 40, pos.y() - 40); }

    static std::vector<std::unique_ptr<TowerPit>> createTowerPits();

private:
    bool hasTower_;
    QRectF rect;
    int index_ = -1;
    bool hasObstacle_ = false;
};

#endif
