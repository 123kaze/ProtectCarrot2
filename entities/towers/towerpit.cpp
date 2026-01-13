#include "towerpit.h"

std::vector<std::unique_ptr<TowerPit>> TowerPit::createTowerPits()
{
    std::vector<std::unique_ptr<TowerPit>> pits;
    pits.reserve(50);

    int index = 0;

    auto addPit = [&](int xTopLeft, int yTopLeft) {
        const QPointF center(xTopLeft + 40.0, yTopLeft + 40.0);
        pits.push_back(std::make_unique<TowerPit>(center, index++));
    };

    for (int i = 0; i < 8; ++i)
    {
        addPit(300 + i * 90, 120);
    }
    for (int i = 0; i < 4; ++i)
    {
        addPit(50 + i * 90, 220);
    }
    for (int i = 0; i < 8; ++i)
    {
        addPit(170 + i * 90, 310);
    }
    for (int i = 0; i < 7; ++i)
    {
        addPit(330 + i * 90, 510);
    }
    for (int i = 0; i < 4; ++i)
    {
        addPit(350 + i * 90, 700);
    }

    for (int i = 0; i < 3; ++i)
    {
        addPit(1000, 200 + i * 90);
    }
    for (int i = 0; i < 3; ++i)
    {
        addPit(1100, 200 + i * 90);
    }
    for (int i = 0; i < 3; ++i)
    {
        addPit(20, 400 + i * 90);
    }
    for (int i = 0; i < 3; ++i)
    {
        addPit(120, 400 + i * 90);
    }

    return pits;
}
