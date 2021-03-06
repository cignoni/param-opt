#include <vcg/complex/complex.h>
#include <wrap/io_trimesh/import.h>
#include <wrap/io_trimesh/export.h>

#include <iostream>
#include <string>

#include <QFileInfo>
#include <QDir>
#include <QString>

#include "mesh.h"
#include "gl_util.h"

bool LoadMesh(Mesh &m, const char *fileName, TextureObjectHandle& textureObject, int &loadMask, std::string &modelName)
{
    m.Clear();
    textureObject = std::make_shared<TextureObject>();
    loadMask = 0;

    QFileInfo fi(fileName);

    if (!fi.exists() || !fi.isReadable()) {
        std::cout << "Unable to read " << fileName << std::endl;
        return false;
    }

    modelName = fi.fileName().toStdString();

    QString wd = QDir::currentPath();
    QDir::setCurrent(fi.absoluteDir().absolutePath());

    int r = tri::io::Importer<Mesh>::Open(m, fi.fileName().toStdString().c_str(), loadMask);
    if (r) {
        std::cout << tri::io::Importer<Mesh>::ErrorMsg(r) << std::endl;
        return false;
    }

    std::cout << "Loaded mesh " << fileName << " (VN " <<  m.VN() << ", FN " << m.FN() << ")" << std::endl;

    for (const string& textureName : m.textures) {
        QFileInfo textureFile(textureName.c_str());
        textureFile.makeAbsolute();
        auto imgptr = std::make_shared<QImage>(textureFile.absoluteFilePath());
        if (!textureFile.exists() || !textureFile.isReadable() || imgptr->isNull()) {
            std::cout << "Unable to load texture file " << textureName.c_str() << std::endl;
            return false;
        }
        textureObject->AddImage(imgptr);
    }

    QDir::setCurrent(wd);
    return true;
}

bool SaveMesh(Mesh &m, const char *fileName, TextureObjectHandle& textureObject)
{
    if (tri::io::Exporter<Mesh>::Save(m, fileName, tri::io::Mask::IOM_WEDGTEXCOORD)) {
        std::cout << "Error saving mesh file " << fileName << std::endl;
        return false;
    }

    QFileInfo fi(fileName);
    assert (fi.exists());

    QString wd = QDir::currentPath();
    QDir::setCurrent(fi.absoluteDir().absolutePath());

    for (std::size_t i = 0; i < textureObject->imgVec.size(); ++i) {
        if(textureObject->imgVec[i]->save(m.textures[i].c_str(), 0, 100) == false) {
            std::cout << "Error saving texture file " << m.textures[0] << std::endl;
            return false;
        }
    }

    QDir::setCurrent(wd);
    return true;
}
