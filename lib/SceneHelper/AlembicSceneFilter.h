#ifndef SCENEHELPER_ALEMBICSCENEFILTER_H_
#define SCENEHELPER_ALEMBICSCENEFILTER_H_

#include <Alembic/AbcGeom/All.h>
#include <string>
#include <map>
#include <regex.h>

class AlembicNode;

class AlembicSceneFilter
{
public:
   
   AlembicSceneFilter();
   AlembicSceneFilter(const std::string &incl, const std::string &excl);
   AlembicSceneFilter(const AlembicSceneFilter &rhs);
   ~AlembicSceneFilter();
   
   AlembicSceneFilter& operator=(const AlembicSceneFilter &rhs);
   
   void set(const std::string &incl, const std::string &excl);
   void reset();
   bool isSet() const;
   
   bool isIncluded(const char *path, bool strict=false) const;
   bool isIncluded(const AlembicNode *node, bool strict=false) const;
   bool isExcluded(const char *path) const;
   bool isExcluded(const AlembicNode *node) const;
   
   bool keep(Alembic::Abc::IObject iObj) const;
   
   inline const std::string& includeExpression() const { return mIncludeFilterStr; }
   inline const std::string& excludeExpression() const { return mExcludeFilterStr; }

protected:
   
   std::string mIncludeFilterStr;
   std::string mExcludeFilterStr;
   regex_t *mIncludeFilter;
   regex_t *mExcludeFilter;
   regex_t mIncludeFilter_;
   regex_t mExcludeFilter_;
   mutable std::map<std::string, bool> mCache;
};


#endif