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
        // 居中绘制
        painter->drawPixmap(pos.x() - pix.width()/2, pos.y() - pix.height()/2, pix);
    }

protected:
    QPointF pos;         // 坐标
    QString spriteName;  // 对应的图片名
};

#endif