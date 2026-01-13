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
    QBuffer* bgmBuffer = nullptr;

    QMediaPlayer* sfxDeathPlayer = nullptr;
    QBuffer* sfxDeathBuffer = nullptr;

    QMediaPlayer* sfxWinPlayer = nullptr;
    QBuffer* sfxWinBuffer = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QAudioOutput* bgmOutput = nullptr;

    QAudioOutput* sfxDeathOutput = nullptr;
    QAudioOutput* sfxWinOutput = nullptr;
#endif

    QRectF towerUpgradeRect;
    QRectF towerSellRect;
    QRectF confirmRect;

    void paintEvent(QPaintEvent* event) override;  // 重写绘图事件
    void mousePressEvent(QMouseEvent* event) override;
};
#endif  // MAINWINDOW_H
