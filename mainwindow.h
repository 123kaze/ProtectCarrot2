#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QTimer>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QMouseEvent>
#include <cstdint>
#include <memory>

#include "game/game_controller.h"

#include <QMediaPlayer>
#include <QBuffer>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#endif
QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

   public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

   private:
    Ui::MainWindow* ui;
    QTimer* gameTimer;

    GameController controller;

    QMediaPlayer* bgmPlayer = nullptr;
    QBuffer* bgmGameBuffer = nullptr;
    QBuffer* bgmMenuBuffer = nullptr;

    QMediaPlayer* sfxDeathPlayer = nullptr;
    QBuffer* sfxDeathBuffer = nullptr;

    QMediaPlayer* sfxClickPlayer = nullptr;
    QBuffer* sfxClickBuffer = nullptr;

    QMediaPlayer* sfxCannonPlayer = nullptr;
    QBuffer* sfxCannonBuffer = nullptr;

    QMediaPlayer* sfxFanPlayer = nullptr;
    QBuffer* sfxFanBuffer = nullptr;

    QMediaPlayer* sfxPoopPlayer = nullptr;
    QBuffer* sfxPoopBuffer = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QAudioOutput* bgmOutput = nullptr;

    QAudioOutput* sfxDeathOutput = nullptr;
    QAudioOutput* sfxClickOutput = nullptr;
    QAudioOutput* sfxCannonOutput = nullptr;
    QAudioOutput* sfxFanOutput = nullptr;
    QAudioOutput* sfxPoopOutput = nullptr;
#endif

    enum class BgmMode
    {
        Menu,
        Game,
    };
    BgmMode bgmMode_ = BgmMode::Menu;

    QRectF towerUpgradeRect;
    QRectF towerSellRect;
    QRectF confirmRect;
    QRectF defeatRestartRect;
    QRectF defeatReturnRect;

    void paintEvent(QPaintEvent* event) override;  // 重写绘图事件
    void mousePressEvent(QMouseEvent* event) override;
};
#endif  // MAINWINDOW_H
