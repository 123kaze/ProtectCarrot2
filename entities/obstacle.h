#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QPointF>
#include <QString>

class Obstacle
{
public:
    Obstacle(int id, const QPointF& pos, const QString& spriteName, int hp, int width, int height);

    int id() const { return id_; }

    void takeDamage(int damage);

    bool isDead() const { return hp_ <= 0; }

    int hp() const { return hp_; }

    QPointF position() const { return pos_; }
    const QString& spriteName() const { return spriteName_; }

    int width() const { return renderW_; }
    int height() const { return renderH_; }

    QPointF centerPosition() const { return QPointF(pos_.x() + renderW_ / 2.0, pos_.y() + renderH_ / 2.0); }

    bool isSelected() const { return selected_; }
    void setSelected(bool selected) { selected_ = selected; }

    bool contains(const QPointF& p) const;

private:
    int id_ = -1;
    QPointF pos_;
    QString spriteName_;

    int hp_ = 0;
    bool selected_ = false;

    int renderW_ = 100;
    int renderH_ = 100;
};

#endif
