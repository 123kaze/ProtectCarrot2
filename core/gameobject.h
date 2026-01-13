#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QPointF>
#include <QPainter>
#include "resourcemanager.h"

class GameObject {
public:
    GameObject(QPointF pos, QString spriteName) 
        : pos(pos), spriteName(spriteName) {}
    
    virtual ~GameObject() = default;

    // 由子类重写：怪物走位、塔找目标等
    virtual void update() = 0; 

    // 绘制自己
    virtual void draw(QPainter *painter) {
        const QPixmap& pix = ResourceManager::instance().getPixmap(spriteName);
        const int w = renderWidth;
        const int targetHeight = pix.isNull() ? 0 : (w * pix.height() / pix.width());

        QRect targetRect(static_cast<int>(pos.x() - w / 2),
                         static_cast<int>(pos.y() - targetHeight / 2),
                         w,
                         targetHeight);
        painter->drawPixmap(targetRect, pix);
    }

protected:
    QPointF pos;         // 坐标
    QString spriteName;  // 对应的图片名
    int renderWidth = 60;
};

#endif