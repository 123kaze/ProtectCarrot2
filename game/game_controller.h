#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <QRectF>
#include <cstdint>
#include <memory>

#include "game_state.h"

class GameWorld;

enum class TowerType;

class GameController
{
public:
    GameController();
    ~GameController();

    void reset();

    void tick(std::int64_t deltaMs);

    bool onClick(const QPointF& scenePos);

    bool togglePause();
    void toggleSpeed();

    const GameWorld& world() const;
    GameWorld& world();

    QRectF speedRect() const { return speedRect_; }
    QRectF pauseRect() const { return pauseRect_; }

    bool isBuildMenuOpen() const { return buildMenuOpen_; }
    QRectF buildBtn1() const { return buildBtn1_; }
    QRectF buildBtn2() const { return buildBtn2_; }
    QRectF buildBtn3() const { return buildBtn3_; }
    QRectF buildBtn4() const { return buildBtn4_; }

    QPointF selectedPitTopLeft() const { return selectedPitTopLeft_; }

    void setHudLayout(int w, int h);

private:
    void closeBuildMenu();
    void openBuildMenuAt(const QPointF& originTopLeft);

    std::unique_ptr<GameWorld> world_;

    QRectF speedRect_;
    QRectF pauseRect_;

    bool buildMenuOpen_ = false;
    QRectF buildBtn1_;
    QRectF buildBtn2_;
    QRectF buildBtn3_;
    QRectF buildBtn4_;

    QPointF selectedPitTopLeft_;
};

#endif
