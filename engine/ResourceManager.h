#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <QHash>
#include <QPixmap>
#include <QString>
#include <memory>

class ResourceManager
{
public:
    static ResourceManager& instance();

    void loadResources(const QString& path);

    const QPixmap& getPixmap(const QString& name);

private:
    ResourceManager();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    QString resolvePixmapPath(const QString& name) const;

    QHash<QString, std::shared_ptr<QPixmap>> pixmapCache;
    QPixmap defaultPixmap;
};

#endif
