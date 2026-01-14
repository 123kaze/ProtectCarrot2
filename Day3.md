 # Day3
 
 ## 今日完成内容
 
 ### 1. 炮塔升级限制：只能升级一次
- 将炮塔升级逻辑限制为“只允许升级一次”。
- 炮塔达到上限后：
  - `upgrade()` 返回 `false`，不会继续升级。
  - 选中该炮塔时升级按钮保持灰暗。

**实现要点**
- 升级次数通过 `Tower::level_` 控制：
  - 初始为 1。
  - 允许升级一次后到 2。
  - 当 `level_ >= 2` 时 `Tower::upgrade()` 直接返回 `false`。
- 升级时同步更新属性（伤害、射程、升级费用、贴图），确保“表现”和“数值”一致。
- `MainWindow::paintEvent()` 中根据 `selectedTower->level() < 2` 和 `world.money() >= selectedTower->upgradeCost()` 判断升级按钮的显示状态。

**涉及文件**
- `entities/towers/tower.cpp`
- `game/game_world.cpp`
- `mainwindow.cpp`

### 2. 升级按钮灰暗逻辑
- UI 绘制阶段根据“是否还有升级次数 + 是否足够金币”决定升级按钮贴图：
  - 可升级：`upgrade_normal_blue`
  - 不可升级（已升级过/金币不足）：`upgrade_normal_gray`

**实现要点**
- `MainWindow::paintEvent()` 在绘制“选中炮塔”的 UI 时：
  - 通过 `selectedTower->level() < 2` 判断是否还有升级次数。
  - 通过 `world.money() >= selectedTower->upgradeCost()` 判断金币是否足够。
  - 两者同时满足才绘制蓝色按钮，否则绘制灰色按钮。
- 点击升级按钮时，最终升级是否成功仍由 `Tower::upgrade()` 返回值决定（UI 只是提示，不作为最终规则来源）。
 
### 3. 萝卜血量（health_10）与受击状态联动（对齐 /.WendyAr）
- 在萝卜旁边绘制生命值贴图 `health_X.png`（满血为 `health_10.png`）。
- 当怪物到达终点导致萝卜扣血时：
  - 萝卜贴图随 HP 变化（`radish_10` -> `radish_9` -> ...）。
  - 生命值贴图同步变化（`health_10` -> `health_9` -> ...）。
- 绘制规则对齐 `/.WendyAr`：
  - 萝卜按 100x100（保持比例）绘制。
  - health 按 60x38（保持比例）绘制：
    - health 底部与萝卜底部对齐。
    - 相对萝卜向左偏一点，并在上层绘制。
  - 萝卜整体向上偏移少量（yOffset = -10）。
 
**涉及文件**
- `entities/radish.h`
- `mainwindow.cpp`（保持 `world.radish().draw(...)`，具体 health 绘制在 Radish 内完成）

**实现要点**
- 受击入口统一为 `Radish::takeDamage(int damage)`：
  - 增加 `damage <= 0` 的输入保护。
  - 更新 `hp` 后立刻更新 `spriteName`（`radish_%1`），并在 `draw()` 中根据 `hp` 绘制对应的 `health_%1`。
- 绘制职责放在 `Radish` 内：
  - `MainWindow` 只调用一次 `world.radish().draw(&painter)`，避免 UI 层反复拼装血量逻辑。
 
### 4. 失败界面增加“再来一次 / 返回”交互（对齐 /.WendyAr）
- 失败界面提供两个透明点击区域：
  - “再来一次”：重置后直接进入倒计时（快速重开一局）。
  - “返回”：回到开始界面。
- 点击区域对齐贴图按钮位置（当前矩形）：
  - `defeatRestartRect = QRectF(420, 450, 300, 80)`
  - `defeatReturnRect  = QRectF(420, 550, 300, 80)`
- 失败界面按钮边框不再绘制（只保留点击判定，不显示调试框）。

**涉及文件**
- `mainwindow.h`
- `mainwindow.cpp`

**实现要点**
- 失败界面的按钮使用“透明点击区域 + 背景贴图”的方式实现：
  - 渲染阶段只绘制 `lose` 背景，不绘制按钮框。
  - 点击判定依赖 `defeatRestartRect/defeatReturnRect.contains(clickPos)`。
- 行为细化：
  - “再来一次”：`controller.reset(); controller.world().startCountdown();`
  - “返回”：`controller.reset();`（回到 Start 状态）

### 5. 音频系统改造：两套BGM切换 + UI点击音效 + 炮塔攻击音效（参考 /.WendyAr）
- BGM 规则（自动切换并循环播放）：
  - 开始界面 / 胜利 / 失败：播放 `mainmanul.mp3`
  - 倒计时 / 游戏进行中 / 暂停：播放 `background_music.mp3`
- UI 点击音效：
  - 建造/升级/暂停/倍速/售卖使用 `click.mp3`
- 炮塔攻击音效（在“实际发射子弹”时触发）：
  - Cannon：`connon.mp3`
  - Fan：`fan.mp3`
  - Poop：`poop.mp3`
- 解耦方式：逻辑层只暴露“一次性事件”，UI 层消费事件并播放音效，避免 GameWorld 与多媒体强耦合。

**实现要点**
- BGM 采用 `BgmMode`（Menu/Game）管理：
  - `MainWindow` 定时器心跳里根据 `GameState` 计算 `desired`，并在切换时重置 position 并 `play()`。
  - Qt6/Qt5 兼容：通过 `QBuffer + QMediaPlayer` 以 `QIODevice` 方式喂数据，避免 `qrc:/` 在部分后端失效。
- SFX 采用“事件消费”方式：
  - `GameWorld` 只设置一次性 flag / event queue（例如 `consumeEnemyDeathSfxRequested()`、`consumeTowerFireSfxEvents()`）。
  - `MainWindow` 消费事件并调用 `restartPlay(player)`，实现“逻辑层不依赖多媒体”。

**涉及文件**
- `res.qrc`
- `game/game_world.h`
- `game/game_world.cpp`
- `entities/towers/tower.h`
- `entities/towers/tower.cpp`
- `mainwindow.h`
- `mainwindow.cpp`

### 6. 敌人贴图朝向：向左移动时水平镜像
- 规则：仅当敌人本帧确实在向左移动时（dx < 0 且 moved）才进行水平镜像；其余方向保持原样。
- 实现：Enemy 记录本帧移动信息（moved / lastMoveDx），渲染时判断 `isMovingLeft()` 并用 `QPainter::scale(-1, 1)` 镜像绘制。

**涉及文件**
- `entities/enemies/enemy.h`
- `entities/enemies/enemy.cpp`
- `mainwindow.cpp`

**实现要点**
- Enemy 在 `update()` 中记录：
  - `movedLastTick_`：本帧是否移动
  - `lastMoveDx_`：本帧 x 方向位移
- `Enemy::isMovingLeft()` 仅在“确实移动且 dx<0”时为真。
- UI 渲染阶段使用 `QPainter` 变换：
  - `translate(rect.x()+rect.width(), rect.y())`
  - `scale(-1, 1)`
  - 在局部 `save/restore`，不影响其它对象绘制。

### 7. 鲁棒性增强：资源缺失提示 + 生命周期安全 + 非法输入防御
- 资源缺失提示：
  - `ResourceManager` 记录加载失败的贴图名列表；启动时预加载关键资源，若缺失则弹窗提示（程序继续运行但可能缺画面）。
- 生命周期安全：
  - 敌人被移除（死亡或到达终点）前，先让仍追踪它的子弹失效并清空指针，避免悬垂指针导致随机崩溃。
- 非法输入防御：
  - `takeDamage()` 对 `damage <= 0` 的输入直接忽略，避免异常输入导致逻辑反向。

**涉及文件**
- `engine/ResourceManager.h`
- `engine/ResourceManager.cpp`
- `game/game_world.cpp`
- `entities/enemies/enemy.cpp`
- `entities/obstacle.cpp`
- `entities/radish.h`
- `mainwindow.cpp`

**实现要点**
- 资源缺失：
  - `ResourceManager::getPixmap()` 加载失败返回 `defaultPixmap`（透明占位），避免空图导致绘制异常。
  - 同时记录 `missingPixmaps_`，启动阶段预加载关键资源后用 `QMessageBox` 一次性提示。
- 生命周期安全：
  - 删除敌人（死亡/到达终点）前，先让追踪它的子弹 `expired_ = true` 且 `enemyTarget_ = nullptr`。
  - 这样避免 `Bullet::update()` 下一帧访问悬垂指针导致随机崩溃。
- 输入非法：
  - `Enemy/Obstacle/Radish::takeDamage()` 对 `damage<=0` 直接 return。

## 今日验证点（运行确认）
- 选中炮塔：
  - 第一次可升级；升级成功后升级按钮变灰。
  - 再点击升级按钮不会继续升级。
- 萝卜被怪物碰到：
  - 萝卜外观与 health 数字贴图同步变化。
- 失败结算界面：
  - 点击“再来一次”会立刻进入倒计时并重新开始。
  - 点击“返回”会回到开始界面。
  - 失败界面不再显示按钮边框（无调试框）。
- 音频：
  - 开始界面/胜利/失败播放 `mainmanul.mp3`；进入倒计时或游戏中切换为 `background_music.mp3`。
  - 点击建造/升级/暂停/倍速/售卖触发 `click.mp3`。
  - 炮塔开火触发对应的攻击音效（cannon/fan/poop）。
- 敌人朝向：
  - 敌人向左移动时贴图水平镜像，其他方向不变。
- 稳定性：
  - 资源缺失时会弹窗提示，程序仍可运行。
  - 敌人被清理后子弹不会因悬垂指针异常乱飞/崩溃。

## AI辅助痕迹标注（交付物要求）
- 本工程在关键实现点加入了 `AI辅助痕迹` 注释，格式为：
  - “此处参考 AI 的某个通用实现思路/草案”
  - “我如何修改/封装/加保护以符合当前架构与鲁棒性要求”
- 典型分布位置：
  - `MainWindow`：定时器驱动 + 音效事件消费 + 镜像渲染。
  - `GameWorld`：删除对象前断开引用（子弹目标失效）。
  - `ResourceManager`：资源失败兜底与缺失资源汇总提示。
  - `Tower/Bullet/Enemy/Radish/Obstacle`：选目标、旋转绘制、hitThreshold、防御式编程等。
