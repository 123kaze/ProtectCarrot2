#include "ResourceManager.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

ResourceManager& ResourceManager::instance()
{
    static ResourceManager manager;
    return manager;
}

ResourceManager::ResourceManager()
{
    defaultPixmap = QPixmap(1, 1);
    defaultPixmap.fill(Qt::transparent);
}

QStringList ResourceManager::missingPixmaps() const
{
    QStringList out = missingPixmaps_.values();
    out.sort();
    return out;
}

void ResourceManager::clearMissingPixmaps()
{
    missingPixmaps_.clear();
}

QString ResourceManager::resolvePixmapPath(const QString& name) const
{
    if (name.startsWith(":/"))
    {
        return name;
    }

    if (name.startsWith('/'))
    {
        return ":" + name;
    }

    if (name.contains('/'))
    {
        return ":/" + name;
    }

    return ":/images/" + name + ".png";
}

const QPixmap& ResourceManager::getPixmap(const QString& name)
{
    if (!pixmapCache.contains(name))
    {
        const QString resolvedPath = resolvePixmapPath(name);
        auto pix = std::make_shared<QPixmap>(resolvedPath);

        if (pix->isNull())
        {
            // AI辅助痕迹：此处参考了 AI 对“资源加载失败要可恢复”的常见建议（返回占位图而不是崩溃/空指针）。
            // 我在此基础上增加了 missingPixmaps_ 记录缺失资源名，便于启动时汇总提示用户。
            qDebug() << "Failed to load image:" << name << "path:" << resolvedPath;
            missingPixmaps_.insert(name);
            return defaultPixmap;
        }

        pixmapCache.insert(name, pix);
    }

    return *(pixmapCache.value(name));
}

void ResourceManager::loadResources(const QString& path)
{
    const QString prefix = path.startsWith(":/") ? path : (":/" + path);

    QDir dir(prefix);
    if (!dir.exists())
    {
        qDebug() << "Resource directory does not exist:" << prefix;
        return;
    }

    QDirIterator it(prefix, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        const QString filePath = it.next();
        QFileInfo fi(filePath);

        const QString ext = fi.suffix().toLower();
        if (ext != "png" && ext != "jpg" && ext != "jpeg" && ext != "bmp" && ext != "gif" &&
            ext != "webp")
        {
            continue;
        }

        const QString key = fi.completeBaseName();

        if (pixmapCache.contains(key))
        {
            continue;
        }

        auto pix = std::make_shared<QPixmap>(filePath);
        if (pix->isNull())
        {
            qDebug() << "Failed to load image from" << filePath;
            continue;
        }

        pixmapCache.insert(key, pix);
    }
}
