#include "game_controller.h"

#include "game_world.h"
#include "../entities/types.h"
 #include "../entities/towers/towerpit.h"

GameController::GameController() : world_(std::make_unique<GameWorld>()) {}

GameController::~GameController() = default;

void GameController::reset()
{
    if (world_)
    {
        world_->reset();
    }
    buildMenuOpen_ = false;
    selectedPitTopLeft_ = QPointF();
}

void GameController::tick(std::int64_t deltaMs)
{
    if (world_)
    {
        world_->update(deltaMs);
    }
}

bool GameController::togglePause()
{
    if (!world_)
    {
        return false;
    }
    closeBuildMenu();
    return world_->togglePause();
}

void GameController::toggleSpeed()
{
    if (!world_)
    {
        return;
    }

    world_->toggleSpeed();
}

const GameWorld& GameController::world() const
{
    return *world_;
}

GameWorld& GameController::world()
{
    return *world_;
}

void GameController::setHudLayout(int w, int)
{
    Q_UNUSED(w);

    // 对齐 /.WendyAr：暂停(60x60)在(1000,10)，倍速(120x120)在(800,10)
    pauseRect_ = QRectF(1000, 10, 60, 60);
    // 需求：倍速按钮（含 x1/x2）显示偏大，适当缩小绘制与点击区域
    speedRect_ = QRectF(820, 0, 100, 80);
}

void GameController::closeBuildMenu()
{
    buildMenuOpen_ = false;
    selectedPitTopLeft_ = QPointF();
}

void GameController::openBuildMenuAt(const QPointF& originTopLeft)
{
    // 对齐 /.WendyAr：四个按钮横排，100x100，gap=10，从坑位左上角展开
    const qreal btn = 100;
    const qreal gap = 10;

    selectedPitTopLeft_ = originTopLeft;
    buildBtn1_ = QRectF(originTopLeft.x(), originTopLeft.y(), btn, btn);
    buildBtn2_ = QRectF(originTopLeft.x() + (btn + gap), originTopLeft.y(), btn, btn);
    buildBtn3_ = QRectF(originTopLeft.x() + 2 * (btn + gap), originTopLeft.y(), btn, btn);
    buildBtn4_ = QRectF(originTopLeft.x() + 3 * (btn + gap), originTopLeft.y(), btn, btn);

    buildMenuOpen_ = true;
}

bool GameController::onClick(const QPointF& scenePos)
{
    if (!world_)
    {
        return false;
    }

    if (world_->state() == GameState::Start)
    {
        world_->startCountdown();
        return true;
    }

    if (world_->state() == GameState::Victory || world_->state() == GameState::Defeat)
    {
        world_->reset();
        return true;
    }

    if (speedRect_.contains(scenePos))
    {
        toggleSpeed();
        return true;
    }

    if (pauseRect_.contains(scenePos))
    {
        togglePause();
        return true;
    }

    if (buildMenuOpen_)
    {
        const auto tryBuild = [&](const QRectF& rect, TowerType type) {
            if (!rect.contains(scenePos))
            {
                return false;
            }
            const bool ok = world_->buildTowerAtSelectedPit(type);
            closeBuildMenu();
            return ok;
        };

        if (tryBuild(buildBtn1_, TowerType::Cannon))
        {
            return true;
        }
        if (tryBuild(buildBtn2_, TowerType::Poop))
        {
            return true;
        }
        if (tryBuild(buildBtn3_, TowerType::Star))
        {
            return true;
        }
        if (tryBuild(buildBtn4_, TowerType::Fan))
        {
            return true;
        }

        closeBuildMenu();
        return true;
    }

    if (world_->handleTowerOrObstacleClick(scenePos))
    {
        return true;
    }

    if (world_->handlePitClick(scenePos))
    {
        // Open build menu at clicked pit top-left.
        for (const auto& pit : world_->pits())
        {
            if (pit && pit->contains(scenePos))
            {
                openBuildMenuAt(pit->topLeft());
                break;
            }
        }
        return true;
    }

    return false;
}
