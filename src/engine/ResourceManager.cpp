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
    defaultPixmap.fill(Qt::transparent);
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
        auto pix = std::make_unique<QPixmap>(resolvedPath);

        if (pix->isNull())
        {
            qDebug() << "Failed to load image:" << name << "path:" << resolvedPath;
            return defaultPixmap;
        }

        pixmapCache.insert(name, std::move(pix));
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
        if (ext != "png" && ext != "jpg" && ext != "jpeg" && ext != "bmp" && ext != "gif" && ext != "webp")
        {
            continue;
        }

        const QString key = fi.completeBaseName();

        if (pixmapCache.contains(key))
        {
            continue;
        }

        auto pix = std::make_unique<QPixmap>(filePath);
        if (pix->isNull())
        {
            qDebug() << "Failed to load image from" << filePath;
            continue;
        }

        pixmapCache.insert(key, std::move(pix));
    }
}
