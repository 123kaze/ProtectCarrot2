#ifndef TARGETING_STRATEGY_H
#define TARGETING_STRATEGY_H

#include <memory>
#include <vector>

class Enemy;

class TargetingStrategy
{
public:
    virtual ~TargetingStrategy() = default;

    virtual Enemy* chooseTarget(const std::vector<Enemy*>& candidates) const = 0;
};

class FurthestProgressStrategy final : public TargetingStrategy
{
public:
    Enemy* chooseTarget(const std::vector<Enemy*>& candidates) const override;
};

#endif
