QT       += core gui widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    engine/ResourceManager.cpp \
    entities/bullets/bullet.cpp \
    entities/enemies/enemy.cpp \
    entities/obstacle.cpp \
    entities/towers/tower.cpp \
    entities/towers/targeting_strategy.cpp \
    entities/towers/towerpit.cpp \
    game/game_controller.cpp \
    game/game_world.cpp \
    game/wave_spawner.cpp \
    main.cpp \
    mainwindow.cpp

INCLUDEPATH += \
    engine \
    core \
    core/Components \
    entities/towers \
    entities \
    entities/enemies \
    entities/bullets \
    game \
    ui \
    ui/widgets \

HEADERS += \
    core/gameobject.h \
    engine/ResourceManager.h \
    engine/resourcemanager.h \
    entities/enemies/emermybase.h \
    entities/enemies/enemy.h \
    entities/bullets/bullet.h \
    entities/obstacle.h \
    entities/radish.h \
    entities/towers/tower.h \
    entities/towers/targeting_strategy.h \
    entities/types.h \
    game/game_controller.h \
    game/game_state.h \
    game/game_world.h \
    game/wave_spawner.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    ProtectCarrot_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    res/images/1.png \
    res/images/2.png \
    res/images/3.png \
    res/images/GO.png \
    res/images/background.png \
    res/images/cannon.png \
    res/images/cannon_bullet.png \
    res/images/cannon_button.png \
    res/images/cannon_upgraded.png \
    res/images/coin.png \
    res/images/double_speed_hover.png \
    res/images/double_speed_normal.png \
    res/images/fan.png \
    res/images/fan_bullet.png \
    res/images/fan_button.png \
    res/images/fan_upgraded.png \
    res/images/health_1.png \
    res/images/health_10.png \
    res/images/health_2.png \
    res/images/health_3.png \
    res/images/health_4.png \
    res/images/health_5.png \
    res/images/health_6.png \
    res/images/health_7.png \
    res/images/health_8.png \
    res/images/health_9.png \
    res/images/level_up.png \
    res/images/lose.png \
    res/images/map21.png \
    res/images/monster1.png \
    res/images/monster1_2.png \
    res/images/monster2.png \
    res/images/monster2_2.png \
    res/images/monster3.png \
    res/images/monster3_2.png \
    res/images/monster4.png \
    res/images/monster4_2.png \
    res/images/monster5.png \
    res/images/monster5_2.png \
    res/images/monsterboss.png \
    res/images/obstacle1.png \
    res/images/obstacle10.png \
    res/images/obstacle2.png \
    res/images/obstacle3.png \
    res/images/obstacle4.png \
    res/images/obstacle5.png \
    res/images/obstacle6.png \
    res/images/obstacle7.png \
    res/images/obstacle8.png \
    res/images/obstacle9.png \
    res/images/pause_hover.png \
    res/images/pause_normal.png \
    res/images/poop.png \
    res/images/poop_bullet.png \
    res/images/poop_button.png \
    res/images/poop_upgraded.png \
    res/images/radish_1.png \
    res/images/radish_1.png.png \
    res/images/radish_10.png \
    res/images/radish_2.png \
    res/images/radish_3.png \
    res/images/radish_4.png \
    res/images/radish_5.png \
    res/images/radish_6.png \
    res/images/radish_7.png \
    res/images/radish_8.png \
    res/images/radish_9.png \
    res/images/remove.png \
    res/images/remove_normal.png \
    res/images/selected_marker.png \
    res/images/star.png \
    res/images/star_bullet.png \
    res/images/star_button.png \
    res/images/star_upgraded.png \
    res/images/tower_pit.png \
    res/images/upgrade_normal_blue.png \
    res/images/upgrade_normal_gray.png \
    res/images/win.png \
    res/sounds/background_music.mp3

RESOURCES += \
    res.qrc
