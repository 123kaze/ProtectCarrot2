#include "mainwindow.h"
#include "engine/resourcemanager.h"
#include "ui_mainwindow.h"

#include <QMouseEvent>
 #include <QMessageBox>

#include "entities/bullets/bullet.h"
#include "entities/enemies/enemy.h"
#include "entities/obstacle.h"
 #include "entities/radish.h"
#include "entities/towers/tower.h"
#include "entities/towers/towerpit.h"
#include "entities/types.h"
#include "game/game_world.h"

#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QCoreApplication>
#include <QFileInfo>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaDevices>
#include <QAudioDevice>
#endif

namespace {

bool loadSoundToBuffer(QBuffer* buffer, QString* inOutPath)
{
    if (!buffer || !inOutPath)
    {
        return false;
    }

    QString path = *inOutPath;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
    {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QStringList candidates = {
            path.startsWith(":/") ? QString() : QStringLiteral(":/") + path,
            QStringLiteral("res/sounds/") + QFileInfo(path).fileName(),
            appDir + QStringLiteral("/res/sounds/") + QFileInfo(path).fileName(),
            appDir + QStringLiteral("/../res/sounds/") + QFileInfo(path).fileName(),
            appDir + QStringLiteral("/../../res/sounds/") + QFileInfo(path).fileName(),
        };

        bool opened = false;
        for (const QString& cand : candidates)
        {
            if (cand.isEmpty() || !QFileInfo::exists(cand))
            {
                continue;
            }
            f.setFileName(cand);
            if (f.open(QIODevice::ReadOnly))
            {
                path = cand;
                opened = true;
                break;
            }
        }
        if (!opened)
        {
            qWarning() << "Sound open failed" << *inOutPath << f.errorString();
            return false;
        }
    }

    buffer->setData(f.readAll());
    f.close();
    buffer->open(QIODevice::ReadOnly);
    *inOutPath = path;
    return true;
}

void restartPlay(QMediaPlayer* player)
{
    if (!player)
    {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    player->stop();
    player->setPosition(0);
    player->play();
#else
    player->stop();
    player->setPosition(0);
    player->play();
#endif
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(1200, 800);

    // 0. 加载游戏资源
    ResourceManager::instance().loadResources("images");

    bgmPlayer = new QMediaPlayer(this);
    bgmBuffer = new QBuffer(this);
    QString bgmPath = QStringLiteral(":/sounds/background_music.mp3");
    if (loadSoundToBuffer(bgmBuffer, &bgmPath))
    {
        qWarning() << "BGM loaded from" << bgmPath << "bytes=" << bgmBuffer->data().size();
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bgmOutput = new QAudioOutput(this);
    bgmOutput->setVolume(0.5);
    bgmPlayer->setAudioOutput(bgmOutput);

    // Qt6: QMediaPlayer may not support qrc:/ URL directly under FFmpeg backend.
    // Feed resource data through QIODevice instead.
    if (bgmBuffer->isOpen())
    {
        bgmPlayer->setSourceDevice(bgmBuffer, QUrl("qrc:/sounds/background_music.mp3"));
    }
    connect(bgmPlayer, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error error, const QString& errorString)
            {
                qWarning() << "BGM error" << error << errorString;
            });
    connect(bgmPlayer, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus status)
            {
                qDebug() << "BGM status" << status;
                if (status == QMediaPlayer::LoadedMedia)
                {
                    bgmPlayer->play();
                }
                else if (status == QMediaPlayer::EndOfMedia)
                {
                    bgmPlayer->setPosition(0);
                    bgmPlayer->play();
                }
                else if (status == QMediaPlayer::InvalidMedia)
                {
                    qWarning() << "BGM invalid media" << bgmPlayer->source();
                }
            });
#else
    // Qt5: also prefer QIODevice to avoid qrc:/ URL issues.
    if (bgmBuffer->isOpen())
    {
        bgmPlayer->setMedia(QMediaContent(), bgmBuffer);
    }
    bgmPlayer->setVolume(40);
    connect(bgmPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this,
            [this](QMediaPlayer::Error error)
            {
                qWarning() << "BGM error" << error << bgmPlayer->errorString();
            });
    connect(bgmPlayer, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus status)
            {
                qDebug() << "BGM status" << status;
                if (status == QMediaPlayer::LoadedMedia)
                {
                    bgmPlayer->play();
                }
                else if (status == QMediaPlayer::EndOfMedia)
                {
                    bgmPlayer->setPosition(0);
                    bgmPlayer->play();
                }
                else if (status == QMediaPlayer::InvalidMedia)
                {
                    qWarning() << "BGM invalid media";
                }
            });
#endif
    bgmPlayer->play();

    // SFX: enemy death
    sfxDeathPlayer = new QMediaPlayer(this);
    sfxDeathBuffer = new QBuffer(this);
    QString deathPath = QStringLiteral(":/sounds/death_moster.mp3");
    if (loadSoundToBuffer(sfxDeathBuffer, &deathPath))
    {
        qWarning() << "SFX death loaded from" << deathPath;
    }

    // SFX: win/click
    sfxWinPlayer = new QMediaPlayer(this);
    sfxWinBuffer = new QBuffer(this);
    QString winPath = QStringLiteral(":/sounds/Win_music.mp3");
    if (loadSoundToBuffer(sfxWinBuffer, &winPath))
    {
        qWarning() << "SFX win loaded from" << winPath;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    sfxDeathOutput = new QAudioOutput(this);
    sfxDeathOutput->setVolume(0.8);
    sfxDeathPlayer->setAudioOutput(sfxDeathOutput);
    if (sfxDeathBuffer->isOpen())
    {
        sfxDeathPlayer->setSourceDevice(sfxDeathBuffer, QUrl("qrc:/sounds/death_moster.mp3"));
    }

    sfxWinOutput = new QAudioOutput(this);
    sfxWinOutput->setVolume(0.8);
    sfxWinPlayer->setAudioOutput(sfxWinOutput);
    if (sfxWinBuffer->isOpen())
    {
        sfxWinPlayer->setSourceDevice(sfxWinBuffer, QUrl("qrc:/sounds/Win_music.mp3"));
    }
#else
    sfxDeathPlayer->setVolume(80);
    if (sfxDeathBuffer->isOpen())
    {
        sfxDeathPlayer->setMedia(QMediaContent(), sfxDeathBuffer);
    }

    sfxWinPlayer->setVolume(80);
    if (sfxWinBuffer->isOpen())
    {
        sfxWinPlayer->setMedia(QMediaContent(), sfxWinBuffer);
    }
#endif

    QTimer::singleShot(1000, this,
                       [this]()
                       {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                           const QAudioDevice dev = QMediaDevices::defaultAudioOutput();
                           qWarning() << "BGM check: source=" << bgmPlayer->source();
                           qWarning() << "BGM check: status=" << bgmPlayer->mediaStatus() << "state="
                                      << bgmPlayer->playbackState() << "error=" << bgmPlayer->error();
                           qWarning() << "BGM check: duration(ms)=" << bgmPlayer->duration() << "pos(ms)="
                                      << bgmPlayer->position();
                           qWarning() << "BGM check: default audio output=" << dev.description() << "isNull="
                                      << dev.isNull();
#else
                           qWarning() << "BGM check: status=" << bgmPlayer->mediaStatus() << "state="
                                      << bgmPlayer->state() << "error=" << bgmPlayer->error() << bgmPlayer->errorString();
                           qWarning() << "BGM check: duration(ms)=" << bgmPlayer->duration() << "pos(ms)="
                                      << bgmPlayer->position();
#endif
                       });

    // AI辅助痕迹：此处参考了 AI 对“控制器驱动 + 定时器心跳”的常见实现建议，
    // 我将原先散落在 MainWindow 的逻辑迁移到 GameController/GameWorld 中，
    // MainWindow 仅保留渲染与输入转发，以降低耦合度。
    controller.reset();
    controller.setHudLayout(width(), height());

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this,
            [this]()
            {
                controller.tick(30);

                // SFX triggers (decoupled: GameWorld only exposes one-shot flags)
                if (controller.world().consumeEnemyDeathSfxRequested())
                {
                    restartPlay(sfxDeathPlayer);
                }
                if (controller.world().consumeVictorySfxRequested())
                {
                    restartPlay(sfxWinPlayer);
                }

                update();
            });

    gameTimer->start(30);
}

MainWindow::~MainWindow()
{
    delete ui;
    // std::vector 里的 unique_ptr 会在 MainWindow 析构时自动释放怪物内存
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return;
    }
    restartPlay(sfxWinPlayer);

    const QPointF clickPos = event->position();

    if (controller.world().state() == GameState::Start)
    {
        const QRectF startRect(450, 250, 200, 100);
        const QRectF rulesRect(1000, 700, 200, 100);

        if (rulesRect.contains(clickPos))
        {
            QMessageBox::information(this, "规则说明", "这里是游戏的规则说明文本。 ");
            return;
        }

        if (startRect.contains(clickPos))
        {
            controller.onClick(clickPos);
            update();
            return;
        }

        return;
    }

    const auto state = controller.world().state();
    if (state == GameState::Victory || state == GameState::Defeat)
    {
        if (confirmRect.contains(clickPos))
        {
            controller.reset();
            update();
        }
        return;
    }

    if (towerUpgradeRect.contains(clickPos))
    {
        controller.world().upgradeSelectedTower();
        update();
        return;
    }
    if (towerSellRect.contains(clickPos))
    {
        controller.world().sellSelectedTower();
        update();
        return;
    }

    // AI辅助痕迹：输入统一转发给 controller，由 controller 决定状态机（GameState）与交互分支。
    controller.onClick(clickPos);
    update();
}

// 4. 每一帧画面长什么样都由这个函数得出
void MainWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 开启抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);

    controller.setHudLayout(width(), height());

    // 对齐 /.WendyAr：开始界面显示 background.png，并提供透明点击区域
    if (controller.world().state() == GameState::Start)
    {
        const QPixmap& bg = ResourceManager::instance().getPixmap("background");
        painter.drawPixmap(0, 0, width(), height(), bg);
        confirmRect = QRectF();
        towerUpgradeRect = QRectF();
        towerSellRect = QRectF();
        return;
    }

    // 画背景地图
    const QPixmap& map = ResourceManager::instance().getPixmap("map21");
    painter.drawPixmap(0, 0, width(), height(), map);

    const QRectF hudSpeedRect = controller.speedRect();
    const QRectF hudPauseRect = controller.pauseRect();

    GameWorld& world = controller.world();

    // 对齐 /.WendyAr：Money 文本
    {
        QFont font;
        font.setFamily("Times New Roman");
        font.setPointSize(16);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(130, 50, QString("Money: %1").arg(world.money()));
    }

    // 对齐 /.WendyAr：波次文本（居中，暂停时显示“暂停中”）
    {
        QString waveText;
        if (world.state() == GameState::Paused)
        {
            waveText = QString("暂停中");
        }
        else
        {
            waveText = QString("%1/%2").arg(world.waveIndex() + 1).arg(world.totalWaves());
        }

        QFont waveFont;
        waveFont.setFamily("Ubuntu");
        waveFont.setPointSize(20);
        waveFont.setBold(true);
        painter.setFont(waveFont);
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, 15, width(), 50), Qt::AlignHCenter | Qt::AlignVCenter, waveText);
    }

    // 对齐 /.WendyAr：暂停/倍速按钮使用贴图
    {
        const bool paused = world.state() == GameState::Paused;
        const QString pauseSprite = paused ? "pause_hover" : "pause_normal";
        const QString speedSprite = (world.speedMultiplier() == 2) ? "double_speed_hover" : "double_speed_normal";
        const QPixmap& pausePix = ResourceManager::instance().getPixmap(pauseSprite);
        const QPixmap& speedPix = ResourceManager::instance().getPixmap(speedSprite);
        painter.drawPixmap(hudPauseRect.toRect(), pausePix);
        painter.drawPixmap(hudSpeedRect.toRect(), speedPix);
    }

    // 对齐需求：默认不显示塔坑（绿色框/坑位贴图），仅在点击打开建造菜单时显示提示
    if (controller.isBuildMenuOpen())
    {
        const QPointF tl = controller.selectedPitTopLeft();
        if (!tl.isNull())
        {
            painter.setPen(QPen(Qt::green, 3));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRectF(tl.x(), tl.y(), 80, 80));
        }
    }

    // obstacles
    for (const auto& obs : world.obstacles())
    {
        if (!obs)
        {
            continue;
        }

        const QPixmap& pix = ResourceManager::instance().getPixmap(obs->spriteName());
        painter.drawPixmap(QRectF(obs->position().x(), obs->position().y(), obs->width(), obs->height()).toRect(), pix);

        if (obs->isSelected())
        {
            const QPixmap& marker = ResourceManager::instance().getPixmap("selected_marker");
            const QPointF markerPos(obs->position().x() + obs->width() / 2 - marker.width() / 2,
                                    obs->position().y() - marker.height());
            painter.drawPixmap(markerPos, marker);
        }
    }

    // towers
    for (const auto& tower : world.towers())
    {
        if (!tower)
        {
            continue;
        }
        const QPixmap& pix = ResourceManager::instance().getPixmap(tower->spriteName());

        if (tower->type() == TowerType::Cannon)
        {
            // AI辅助痕迹：此处参考 /.WendyAr 的绘制方式（translate 到中心 -> rotate -> 绘制居中矩形），
            // 我将旋转角度从 Tower 的状态中读取，避免 MainWindow 直接参与“面向目标”的计算。
            painter.save();
            painter.translate(tower->position());
            painter.rotate(tower->rotationDeg());
            // cannon 保持原贴图比例的1/2,二级三级是双倍
            double scale = (tower->level() <= 1) ? 0.5 : 2.0;
            if (tower->level() == 0)
            {
                scale = 0.5;
            }
            else if (tower -> level() == 1)
            {
                scale = 0.5;
            }
            else{
                scale = 2.0;
            }
        
            const int w = static_cast<int>((pix.isNull() ? 80 : pix.width()) * scale);
            const int h = static_cast<int>((pix.isNull() ? 80 : pix.height()) * scale);
            painter.drawPixmap(QRect(-w / 2, -h / 2, w, h), pix);
            painter.restore();
        }
        else
        {
            const QRectF rect(tower->position().x() - 40, tower->position().y() - 40, 80, 80);
            painter.drawPixmap(rect.toRect(), pix);
        }
    }

    // bullets
    for (const auto& b : world.bullets())
    {
        if (!b)
        {
            continue;
        }
        const QPixmap& pix = ResourceManager::instance().getPixmap(b->spriteName());
        int w = pix.isNull() ? 30 : pix.width();
        int h = pix.isNull() ? 30 : pix.height();

        // cannon 子弹变大一点
        if (b->spriteName() == QString("cannon_bullet"))
        {
            w = static_cast<int>(w * 1.4);
            h = static_cast<int>(h * 1.4);
        }
        const QRectF rect(b->position().x() - w / 2.0, b->position().y() - h / 2.0, w, h);

        // AI辅助痕迹：此处的“子弹旋转绘制”参考了 AI 提供的 QPainter 变换用法，
        // 我改成以子弹中心点为旋转原点，并保证 save/restore 成对出现，避免影响其它绘制。
        if (b->shouldRotate())
        {
            painter.save();
            painter.translate(b->position());
            painter.rotate(b->rotationDeg());
            painter.drawPixmap(QRect(-w / 2, -h / 2, w, h), pix);
            painter.restore();
        }
        else
        {
            painter.drawPixmap(rect.toRect(), pix);
        }
    }

    // enemies
    for (const auto& e : world.enemies())
    {
        if (!e)
        {
            continue;
        }

        const QPixmap& pix = ResourceManager::instance().getPixmap(e->spriteName());
        const bool isBoss = (e->type() == EnemyType::MonsterBoss);
        const qreal sz = isBoss ? 120 : 80;

        // 对齐 /.WendyAr：敌人 position 作为左上角坐标绘制
        // 同时使用 renderPosition() 注入移动时的轻微上下摆动动画
        const QPointF rp = e->renderPosition();
        const QRectF rect(rp.x(), rp.y(), sz, sz);
        painter.drawPixmap(rect.toRect(), pix);
    }

    // radish
    world.radish().draw(&painter);

    // build menu
    if (controller.isBuildMenuOpen())
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 120));
        const QRectF menuRect = controller.buildBtn1().adjusted(-6, -6, 6, 6)
                                    .united(controller.buildBtn4().adjusted(-6, -6, 6, 6));
        painter.drawRoundedRect(menuRect, 8, 8);

        painter.drawPixmap(controller.buildBtn1().toRect(), ResourceManager::instance().getPixmap("cannon_button"));
        painter.drawPixmap(controller.buildBtn2().toRect(), ResourceManager::instance().getPixmap("poop_button"));
        painter.drawPixmap(controller.buildBtn3().toRect(), ResourceManager::instance().getPixmap("star_button"));
        painter.drawPixmap(controller.buildBtn4().toRect(), ResourceManager::instance().getPixmap("fan_button"));
    }

    // selected tower overlay
    towerUpgradeRect = QRectF();
    towerSellRect = QRectF();
    if (world.selected().kind == SelectedObject::Kind::Tower)
    {
        const int selectedId = world.selected().id;
        const Tower* selectedTower = nullptr;
        for (const auto& t : world.towers())
        {
            if (t && t->id() == selectedId)
            {
                selectedTower = t.get();
                break;
            }
        }

        if (selectedTower)
        {
            const bool canUpgrade = world.money() >= selectedTower->upgradeCost();
            const QString upSprite = canUpgrade ? "upgrade_normal_blue" : "upgrade_normal_gray";
            const QPixmap& upPix = ResourceManager::instance().getPixmap(upSprite);
            const QPixmap& rmPix = ResourceManager::instance().getPixmap("remove_normal");

            const QPointF base = selectedTower->position();
            towerUpgradeRect = QRectF(base.x() - 25, base.y() - 80, 50, 50);
            towerSellRect = QRectF(base.x() - 25, base.y() + 40, 50, 50);

            painter.drawPixmap(towerUpgradeRect.toRect(), upPix);
            painter.drawPixmap(towerSellRect.toRect(), rmPix);
        }
    }

    // countdown
    if (world.state() == GameState::Countdown)
    {
        const std::int64_t ms = world.countdownMsLeft();
        QString sprite;
        if (ms > 3000)
            sprite = "3";
        else if (ms > 2000)
            sprite = "2";
        else if (ms > 1000)
            sprite = "1";
        else
            sprite = "GO";

        const QPixmap& pix = ResourceManager::instance().getPixmap(sprite);
        const QPointF p((width() - pix.width()) / 2.0, (height() - pix.height()) / 2.0);
        painter.drawPixmap(p, pix);
    }

    // victory/defeat overlay
    confirmRect = QRectF(400, 600, 200, 100);
    if (world.state() == GameState::Victory)
    {
        painter.drawPixmap(0, 0, width(), height(), ResourceManager::instance().getPixmap("win"));
        painter.setBrush(QColor(255, 255, 255, 0));
        painter.drawRect(confirmRect);
    }
    else if (world.state() == GameState::Defeat)
    {
        painter.drawPixmap(0, 0, width(), height(), ResourceManager::instance().getPixmap("lose"));
        painter.setBrush(QColor(255, 255, 255, 0));
        painter.drawRect(confirmRect);
    }
}
