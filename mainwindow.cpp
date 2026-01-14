#include "mainwindow.h"
#include "engine/ResourceManager.h"
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

    // AI辅助痕迹：此处参考了 AI 对“启动阶段预加载关键资源 + 缺失即提示”的建议，
    // 我将缺失资源收集逻辑封装进 ResourceManager，并在 UI 层用 QMessageBox 汇总提示，避免运行中黑屏且便于验收。
    // 启动时预加载关键贴图并提示缺失项，避免运行中出现黑块/不可见。
    {
        ResourceManager::instance().clearMissingPixmaps();
        const QStringList essential = {
            "background",
            "map21",
            "win",
            "lose",
            "pause_normal",
            "pause_hover",
            "double_speed_normal",
            "double_speed_hover",
            "cannon",
            "poop",
            "star",
            "fan",
            "cannon_bullet",
            "poop_bullet",
            "star_bullet",
            "fan_bullet",
            "monster1",
            "monster1_2",
            "monster2",
            "monster2_2",
            "monster3",
            "monster3_2",
            "monster4",
            "monster4_2",
            "monster5",
            "monster5_2",
            "monsterboss",
            "radish_10",
            "health_10",
        };

        for (const QString& k : essential)
        {
            ResourceManager::instance().getPixmap(k);
        }

        const QStringList missing = ResourceManager::instance().missingPixmaps();
        if (!missing.isEmpty())
        {
            const int maxShow = 12;
            QStringList show = missing.mid(0, maxShow);
            QString msg = QStringLiteral("以下资源加载失败（程序会继续运行，但画面可能缺失）：\n\n%1")
                              .arg(show.join("\n"));
            if (missing.size() > maxShow)
            {
                msg += QStringLiteral("\n\n... 还有 %1 个未显示")
                           .arg(missing.size() - maxShow);
            }
            QMessageBox::warning(this, QStringLiteral("资源缺失"), msg);
        }
    }

    bgmPlayer = new QMediaPlayer(this);
    bgmGameBuffer = new QBuffer(this);
    bgmMenuBuffer = new QBuffer(this);

    QString bgmGamePath = QStringLiteral(":/sounds/background_music.mp3");
    if (loadSoundToBuffer(bgmGameBuffer, &bgmGamePath))
    {
        qWarning() << "BGM(game) loaded from" << bgmGamePath << "bytes=" << bgmGameBuffer->data().size();
    }

    QString bgmMenuPath = QStringLiteral(":/sounds/mainmanul.mp3");
    if (loadSoundToBuffer(bgmMenuBuffer, &bgmMenuPath))
    {
        qWarning() << "BGM(menu) loaded from" << bgmMenuPath << "bytes=" << bgmMenuBuffer->data().size();
    }

    const auto applyBgmMode = [this]() {
        if (!bgmPlayer)
        {
            return;
        }
        QBuffer* buf = (bgmMode_ == BgmMode::Game) ? bgmGameBuffer : bgmMenuBuffer;
        if (!buf || !buf->isOpen())
        {
            return;
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QUrl url = (bgmMode_ == BgmMode::Game) ? QUrl("qrc:/sounds/background_music.mp3")
                                                    : QUrl("qrc:/sounds/mainmanul.mp3");
        bgmPlayer->stop();
        bgmPlayer->setSourceDevice(buf, url);
        bgmPlayer->setPosition(0);
        bgmPlayer->play();
#else
        bgmPlayer->stop();
        bgmPlayer->setMedia(QMediaContent(), buf);
        bgmPlayer->setPosition(0);
        bgmPlayer->play();
#endif
    };
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bgmOutput = new QAudioOutput(this);
    bgmOutput->setVolume(0.5);
    bgmPlayer->setAudioOutput(bgmOutput);

    // QMediaPlayer may not support qrc:/ URL directly under FFmpeg backend.
    // Feed resource data through QIODevice instead.
    applyBgmMode();
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
    // also prefer QIODevice to avoid qrc:/ URL issues.
    applyBgmMode();
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

    // enemy death
    sfxDeathPlayer = new QMediaPlayer(this);
    sfxDeathBuffer = new QBuffer(this);
    QString deathPath = QStringLiteral(":/sounds/death_moster.mp3");
    if (loadSoundToBuffer(sfxDeathBuffer, &deathPath))
    {
        qWarning() << "SFX death loaded from" << deathPath;
    }

    // UI click
    sfxClickPlayer = new QMediaPlayer(this);
    sfxClickBuffer = new QBuffer(this);
    QString clickPath = QStringLiteral(":/sounds/click.mp3");
    if (loadSoundToBuffer(sfxClickBuffer, &clickPath))
    {
        qWarning() << "SFX click loaded from" << clickPath;
    }

    // tower attacks
    sfxCannonPlayer = new QMediaPlayer(this);
    sfxCannonBuffer = new QBuffer(this);
    QString cannonPath = QStringLiteral(":/sounds/connon.mp3");
    if (loadSoundToBuffer(sfxCannonBuffer, &cannonPath))
    {
        qWarning() << "SFX cannon loaded from" << cannonPath;
    }

    sfxFanPlayer = new QMediaPlayer(this);
    sfxFanBuffer = new QBuffer(this);
    QString fanPath = QStringLiteral(":/sounds/fan.mp3");
    if (loadSoundToBuffer(sfxFanBuffer, &fanPath))
    {
        qWarning() << "SFX fan loaded from" << fanPath;
    }

    sfxPoopPlayer = new QMediaPlayer(this);
    sfxPoopBuffer = new QBuffer(this);
    QString poopPath = QStringLiteral(":/sounds/poop.mp3");
    if (loadSoundToBuffer(sfxPoopBuffer, &poopPath))
    {
        qWarning() << "SFX poop loaded from" << poopPath;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    sfxDeathOutput = new QAudioOutput(this);
    sfxDeathOutput->setVolume(0.8);
    sfxDeathPlayer->setAudioOutput(sfxDeathOutput);
    if (sfxDeathBuffer->isOpen())
    {
        sfxDeathPlayer->setSourceDevice(sfxDeathBuffer, QUrl("qrc:/sounds/death_moster.mp3"));
    }

    sfxClickOutput = new QAudioOutput(this);
    sfxClickOutput->setVolume(0.8);
    sfxClickPlayer->setAudioOutput(sfxClickOutput);
    if (sfxClickBuffer->isOpen())
    {
        sfxClickPlayer->setSourceDevice(sfxClickBuffer, QUrl("qrc:/sounds/click.mp3"));
    }

    sfxCannonOutput = new QAudioOutput(this);
    sfxCannonOutput->setVolume(0.8);
    sfxCannonPlayer->setAudioOutput(sfxCannonOutput);
    if (sfxCannonBuffer->isOpen())
    {
        sfxCannonPlayer->setSourceDevice(sfxCannonBuffer, QUrl("qrc:/sounds/connon.mp3"));
    }

    sfxFanOutput = new QAudioOutput(this);
    sfxFanOutput->setVolume(0.8);
    sfxFanPlayer->setAudioOutput(sfxFanOutput);
    if (sfxFanBuffer->isOpen())
    {
        sfxFanPlayer->setSourceDevice(sfxFanBuffer, QUrl("qrc:/sounds/fan.mp3"));
    }

    sfxPoopOutput = new QAudioOutput(this);
    sfxPoopOutput->setVolume(0.8);
    sfxPoopPlayer->setAudioOutput(sfxPoopOutput);
    if (sfxPoopBuffer->isOpen())
    {
        sfxPoopPlayer->setSourceDevice(sfxPoopBuffer, QUrl("qrc:/sounds/poop.mp3"));
    }
#else
    sfxDeathPlayer->setVolume(80);
    if (sfxDeathBuffer->isOpen())
    {
        sfxDeathPlayer->setMedia(QMediaContent(), sfxDeathBuffer);
    }

    sfxClickPlayer->setVolume(80);
    if (sfxClickBuffer->isOpen())
    {
        sfxClickPlayer->setMedia(QMediaContent(), sfxClickBuffer);
    }

    sfxCannonPlayer->setVolume(80);
    if (sfxCannonBuffer->isOpen())
    {
        sfxCannonPlayer->setMedia(QMediaContent(), sfxCannonBuffer);
    }

    sfxFanPlayer->setVolume(80);
    if (sfxFanBuffer->isOpen())
    {
        sfxFanPlayer->setMedia(QMediaContent(), sfxFanBuffer);
    }

    sfxPoopPlayer->setVolume(80);
    if (sfxPoopBuffer->isOpen())
    {
        sfxPoopPlayer->setMedia(QMediaContent(), sfxPoopBuffer);
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

                // BGM switch by GameState
                const GameState s = controller.world().state();
                const bool inGame = (s == GameState::Countdown || s == GameState::Running || s == GameState::Paused);
                const BgmMode desired = inGame ? BgmMode::Game : BgmMode::Menu;
                if (desired != bgmMode_)
                {
                    bgmMode_ = desired;

                    QBuffer* buf = (bgmMode_ == BgmMode::Game) ? bgmGameBuffer : bgmMenuBuffer;
                    if (bgmPlayer && buf && buf->isOpen())
                    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                        const QUrl url = (bgmMode_ == BgmMode::Game) ? QUrl("qrc:/sounds/background_music.mp3")
                                                                    : QUrl("qrc:/sounds/mainmanul.mp3");
                        bgmPlayer->stop();
                        bgmPlayer->setSourceDevice(buf, url);
                        bgmPlayer->setPosition(0);
                        bgmPlayer->play();
#else
                        bgmPlayer->stop();
                        bgmPlayer->setMedia(QMediaContent(), buf);
                        bgmPlayer->setPosition(0);
                        bgmPlayer->play();
#endif
                    }
                }

                // AI辅助痕迹：此处参考了 AI 对“游戏逻辑层只产生事件、UI 层负责播放音效”的解耦建议，
                // 我让 GameWorld 仅暴露一次性标志/事件队列（consumeXXX），MainWindow 统一消费并播放，避免逻辑层依赖 QMediaPlayer。
                // SFX triggers (GameWorld only exposes one-shot flags)
                if (controller.world().consumeEnemyDeathSfxRequested())
                {
                    restartPlay(sfxDeathPlayer);
                }

                // Tower fire SFX events
                for (TowerType t : controller.world().consumeTowerFireSfxEvents())
                {
                    if (t == TowerType::Cannon)
                    {
                        restartPlay(sfxCannonPlayer);
                    }
                    else if (t == TowerType::Fan)
                    {
                        restartPlay(sfxFanPlayer);
                    }
                    else if (t == TowerType::Poop)
                    {
                        restartPlay(sfxPoopPlayer);
                    }
                }

                update();
            });

    gameTimer->start(30);
}

MainWindow::~MainWindow()
{
    delete ui;
    // 里的 unique_ptr 会在 MainWindow 析构时自动释放怪物内存
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return;
    }

    const QPointF clickPos = event->position();

    bool shouldPlayClick = false;

    if (controller.world().state() == GameState::Start)
    {
        const QRectF startRect(450, 250, 200, 100);
        const QRectF rulesRect(1000, 700, 200, 100);
        if (startRect.contains(clickPos) || rulesRect.contains(clickPos))
        {
            shouldPlayClick = true;
        }
    }
    else
    {
        if (towerUpgradeRect.contains(clickPos) || towerSellRect.contains(clickPos))
        {
            shouldPlayClick = true;
        }
        if (controller.speedRect().contains(clickPos) || controller.pauseRect().contains(clickPos))
        {
            shouldPlayClick = true;
        }
        if (controller.isBuildMenuOpen()
            && (controller.buildBtn1().contains(clickPos) || controller.buildBtn2().contains(clickPos)
                || controller.buildBtn3().contains(clickPos) || controller.buildBtn4().contains(clickPos)))
        {
            shouldPlayClick = true;
        }
    }

    if (shouldPlayClick)
    {
        restartPlay(sfxClickPlayer);
    }

    if (controller.world().state() == GameState::Start)
    {
        const QRectF startRect(450, 250, 200, 100);
        const QRectF rulesRect(1000, 700, 200, 100);

        if (rulesRect.contains(clickPos))
        {
            QMessageBox::information(this, "规则说明", "1. 点击空地建塔\n"
        "2. 击败怪物获得金币\n"
        "3. 怪物到达终点萝卜扣血\n"
        "4. 萝卜血量为0则失败\n");
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
    if (state == GameState::Victory)
    {
        if (confirmRect.contains(clickPos))
        {
            controller.reset();
            update();
        }
        return;
    }

    if (state == GameState::Defeat)
    {
        if (defeatRestartRect.contains(clickPos))
        {
            controller.reset();
            controller.world().startCountdown();
            update();
            return;
        }
        if (defeatReturnRect.contains(clickPos))
        {
            controller.reset();
            update();
            return;
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

    // 输入统一转发给 controller，由 controller 决定状态机（GameState）与交互分支。
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

    // 开始界面显示 background.png，并提供透明点击区域
    if (controller.world().state() == GameState::Start)
    {
        const QPixmap& bg = ResourceManager::instance().getPixmap("background");
        painter.drawPixmap(0, 0, width(), height(), bg);
        confirmRect = QRectF();
        defeatRestartRect = QRectF();
        defeatReturnRect = QRectF();
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

    // Money 文本
    {
        QFont font;
        font.setFamily("Times New Roman");
        font.setPointSize(16);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(130, 50, QString("Money: %1").arg(world.money()));
    }

    // 波次文本（居中，暂停时显示“暂停中”）
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

    // 暂停/倍速按钮使用贴图
    {
        const bool paused = world.state() == GameState::Paused;
        const QString pauseSprite = paused ? "pause_hover" : "pause_normal";
        const QString speedSprite = (world.speedMultiplier() == 2) ? "double_speed_hover" : "double_speed_normal";
        const QPixmap& pausePix = ResourceManager::instance().getPixmap(pauseSprite);
        const QPixmap& speedPix = ResourceManager::instance().getPixmap(speedSprite);
        painter.drawPixmap(hudPauseRect.toRect(), pausePix);
        painter.drawPixmap(hudSpeedRect.toRect(), speedPix);
    }

    // 默认不显示塔坑（绿色框/坑位贴图），仅在点击打开建造菜单时显示提示
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
            // 此处参考了绘制方式（translate 到中心 -> rotate -> 绘制居中矩形），
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

        // 敌人 position 作为左上角坐标绘制
        // 同时使用 renderPosition() 注入移动时的轻微上下摆动动画
        const QPointF rp = e->renderPosition();
        const QRectF rect(rp.x(), rp.y(), sz, sz);
        // AI辅助痕迹：此处参考了 AI 提供的“通过 QPainter::scale(-1,1) 实现水平镜像”的绘制方式，
        // 我将“是否向左移动”的判定封装进 Enemy（isMovingLeft），UI 仅根据状态渲染，避免 UI 参与移动逻辑。
        if (e->isMovingLeft())
        {
            painter.save();
            painter.translate(rect.x() + rect.width(), rect.y());
            painter.scale(-1.0, 1.0);
            painter.drawPixmap(QRectF(0, 0, rect.width(), rect.height()).toRect(), pix);
            painter.restore();
        }
        else
        {
            painter.drawPixmap(rect.toRect(), pix);
        }
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
            const bool canUpgrade = (selectedTower->level() < 2) && (world.money() >= selectedTower->upgradeCost());
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
    confirmRect = QRectF(500, 520, 200, 80);
    defeatRestartRect = QRectF();
    defeatReturnRect = QRectF();
    if (world.state() == GameState::Victory)
    {
        painter.drawPixmap(0, 0, width(), height(), ResourceManager::instance().getPixmap("win"));
        painter.setBrush(QColor(255, 255, 255, 0));
        painter.setPen(Qt::NoPen);
        painter.drawRect(confirmRect);
    }
    else if (world.state() == GameState::Defeat)
    {
        painter.drawPixmap(0, 0, width(), height(), ResourceManager::instance().getPixmap("lose"));

        // 失败界面两个按钮区域（重新开始 / 返回）
        defeatRestartRect = QRectF(420, 450, 300, 80);
        defeatReturnRect = QRectF(420, 550, 300, 80);

        painter.setBrush(QColor(255, 255, 255, 0));
    }
}
