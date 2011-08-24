#ifndef AORTMESHPARSER_H
#define AORTMESHPARSER_H

#include <OGRE/OgrePrerequisites.h>

namespace Aort {
  class Material;
  class Triangle;

  class MeshParserPrivate;

  class MeshParser {
  public:
    MeshParser(const Ogre::Entity *entity);
    ~MeshParser();

    const size_t triangleCount() const;
    Triangle **triangles() const;

  private:
    MeshParserPrivate *d;
  };
}

#endif // AORTMESHPARSER_H
