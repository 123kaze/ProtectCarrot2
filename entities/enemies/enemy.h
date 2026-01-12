#include "../../core/gameobject.h"
#include <QList>
#include <QPointF>
#include <cmath>

class Enemy : public GameObject {
public:
    Enemy(QList<QPointF> path) : GameObject(path[0], "monster1"), waypoints(path) {
        currentPointIdx = 0;
        speed = 2.0; 
    }

    void update() override {
        if (currentPointIdx >= waypoints.size()) return;

        QPointF target = waypoints[currentPointIdx];
        
        // 1. 计算移动向量
        qreal dx = target.x() - pos.x();
        qreal dy = target.y() - pos.y();
        qreal distance = std::sqrt(dx * dx + dy * dy);

        if (distance < speed) {
            // 2. 到达当前拐点，转向下一个点
            pos = target;
            currentPointIdx++;
        } else {
            // 3. 利用单位向量保证匀速移动
            pos.setX(pos.x() + speed * (dx / distance));
            pos.setY(pos.y() + speed * (dy / distance));
        }
        
        // 4. 动画切换逻辑
        static int frameCounter = 0;
        frameCounter++;
        spriteName = (frameCounter % 20 < 10) ? "monster1" : "monster1_2";
    }

private:
    QList<QPointF> waypoints; // 路径点序列
    int currentPointIdx;      // 当前正在往第几个点走
    float speed;              // 移动速度
};
