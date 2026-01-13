#include "targeting_strategy.h"

#include "../enemies/enemy.h"

Enemy* FurthestProgressStrategy::chooseTarget(const std::vector<Enemy*>& candidates) const
{
    Enemy* best = nullptr;
    double bestProgress = -1.0;

    for (Enemy* e : candidates)
    {
        if (!e || e->isDead())
        {
            continue;
        }

        const double p = e->progress();
        if (p > bestProgress)
        {
            bestProgress = p;
            best = e;
        }
    }

    return best;
}
