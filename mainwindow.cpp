#include "mainwindow.h"

#include "engine/resourcemanager.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 0. 加载游戏资源
    ResourceManager::instance().loadResources("images");

    // 1. 设置路径点 (参考你的地图 map21.png，坐标需要根据实际路径微调)
    QList<QPointF> path;
    path << QPointF(0, 450) << QPointF(200, 450) << QPointF(200, 150) << QPointF(500, 150)
         << QPointF(500, 600) << QPointF(800, 600);

    // 2. 创建测试怪物 (使用智能指针)
    monsters.push_back(std::make_unique<Enemy>(path));

    // 3. 初始化并启动游戏主循环 (心跳)
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this,
            [this]()
            {
                // 逻辑更新：遍历所有怪物，执行它们的 update 动作
                for (auto& monster : monsters)
                {
                    monster->update();
                }

                // 渲染更新：强制窗口重绘，触发 paintEvent
                update();
            });

    gameTimer->start(30);  // 30ms 刷新一次，约 33 帧/秒
}

MainWindow::~MainWindow()
{
    delete ui;
    // std::vector 里的 unique_ptr 会在 MainWindow 析构时自动释放怪物内存
}

// 4. 每一帧画面长什么样都由这个函数得出
void MainWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 开启抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);

    // 画背景地图
    painter.drawPixmap(0, 0, width(), height(), ResourceManager::instance().getPixmap("map21"));

    // 画所有怪物
    for (auto& monster : monsters)
    {
        monster->draw(&painter);
    }
}