#pragma once
#include <map>
#include <set>
#include <BWAPI.h>
#include <Arbitrator.h>
#include <BWTA.h>
#include "Base.h"
class BuildOrderManager;
class BorderManager;
class BaseManager
{
  public:
  BaseManager();
  void setBuildOrderManager(BuildOrderManager* builder);
  void setBorderManager(BorderManager* borderManager);
  void update();
  Base* getBase(BWTA::BaseLocation* location);
  void expand(int priority = 100);
  void expand(BWTA::BaseLocation* location, int priority = 100);
  std::set<Base*> getActiveBases() const;
  std::set<Base*> getAllBases() const;

  std::string getName();
  void onRemoveUnit(BWAPI::Unit* unit);

  private:
  void addBase(BWTA::BaseLocation* location);
  BuildOrderManager* builder;
  BorderManager* borderManager;
  std::map<BWTA::BaseLocation*,Base*> location2base;
  std::set<Base*> allBases;
};