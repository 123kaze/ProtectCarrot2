#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPainter>
#include <vector>
#include <memory>
#include "entities/enemies/enemy.h"

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
    QTimer *gameTimer;
    std::vector<std::unique_ptr<Enemy>> monsters;
    void paintEvent(QPaintEvent *event) override; // 重写绘图事件
};
#endif  // MAINWINDOW_H
