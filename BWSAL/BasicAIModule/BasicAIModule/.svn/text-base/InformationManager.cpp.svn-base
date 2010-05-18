#include <InformationManager.h>
#include "Util.h"
InformationManager::InformationManager()
{
  buildTime[*BWAPI::Broodwar->enemy()->getRace().getCenter()]=0;
  buildTime[*BWAPI::Broodwar->enemy()->getRace().getWorker()]=0;
  if (BWAPI::Broodwar->enemy()->getRace()==BWAPI::Races::Zerg)
  {
    buildTime[BWAPI::UnitTypes::Zerg_Larva]=0;
    buildTime[BWAPI::UnitTypes::Zerg_Overlord]=0;
  }
}
void InformationManager::onUnitShow(BWAPI::Unit* unit)
{
  savedData[unit].exists=true;
  if (!BWAPI::Broodwar->self()->isEnemy(unit->getPlayer())) return;
  int time=BWAPI::Broodwar->getFrameCount();
  BWAPI::UnitType type=unit->getType();
  updateBuildTime(type,time-type.buildTime());
  if (unit->getType().isResourceDepot())
  {
    BWTA::BaseLocation* b=BWTA::getNearestBaseLocation(unit->getTilePosition());
    enemyBases.insert(b);
    enemyBaseCenters[b]=unit;
  }
}
void InformationManager::onUnitHide(BWAPI::Unit* unit)
{
  savedData[unit].player=unit->getPlayer();
  savedData[unit].type=unit->getType();
  savedData[unit].position=unit->getPosition();
  savedData[unit].lastSeenTime=BWAPI::Broodwar->getFrameCount();
}
void InformationManager::onUnitDestroy(BWAPI::Unit* unit)
{
  savedData[unit].exists=false;
  savedData[unit].player=unit->getPlayer();
  savedData[unit].type=unit->getType();
  savedData[unit].position=unit->getPosition();
  savedData[unit].lastSeenTime=BWAPI::Broodwar->getFrameCount();
  if (!BWAPI::Broodwar->self()->isEnemy(unit->getPlayer())) return;
  if (unit->getType().isResourceDepot())
  {
    BWTA::BaseLocation* b=BWTA::getNearestBaseLocation(unit->getTilePosition());
    if (this->enemyBaseCenters[b]==unit)
    {
      this->enemyBases.erase(b);
      this->enemyBaseCenters.erase(b);
    }
  }
}

BWAPI::Player* InformationManager::getPlayer(BWAPI::Unit* unit) const
{
  if (unit->exists())
    return unit->getPlayer();
  std::map<BWAPI::Unit*,UnitData>::const_iterator i=savedData.find(unit);
  if (i==savedData.end())
    return NULL;
  return (*i).second.player;
}

BWAPI::UnitType InformationManager::getType(BWAPI::Unit* unit) const
{
  if (unit->exists())
    return unit->getType();
  std::map<BWAPI::Unit*,UnitData>::const_iterator i=savedData.find(unit);
  if (i==savedData.end())
    return BWAPI::UnitTypes::None;
  return (*i).second.type;
}

BWAPI::Position InformationManager::getLastPosition(BWAPI::Unit* unit) const
{
  if (unit->exists())
    return unit->getPosition();
  std::map<BWAPI::Unit*,UnitData>::const_iterator i=savedData.find(unit);
  if (i==savedData.end())
    return BWAPI::Positions::None;
  return (*i).second.position;
}

int InformationManager::getLastSeenTime(BWAPI::Unit* unit) const
{
  if (unit->exists())
    return BWAPI::Broodwar->getFrameCount();
  std::map<BWAPI::Unit*,UnitData>::const_iterator i=savedData.find(unit);
  if (i==savedData.end())
    return -1;
  return (*i).second.lastSeenTime;
}

bool InformationManager::exists(BWAPI::Unit* unit) const
{
  if (unit->exists())
    return true;
  std::map<BWAPI::Unit*,UnitData>::const_iterator i=savedData.find(unit);
  if (i==savedData.end())
    return false;
  return (*i).second.exists;
}

bool InformationManager::enemyHasBuilt(BWAPI::UnitType type) const
{
  return (buildTime.find(type)!=buildTime.end());
}

int InformationManager::getBuildTime(BWAPI::UnitType type) const
{
  std::map<BWAPI::UnitType, int>::const_iterator i=buildTime.find(type);
  if (i==buildTime.end())
    return -1;
  return i->second;
}

const std::set<BWTA::BaseLocation*>& InformationManager::getEnemyBases() const
{
  return this->enemyBases;
}

void InformationManager::updateBuildTime(BWAPI::UnitType type, int time)
{
  std::map<BWAPI::UnitType, int>::iterator i=buildTime.find(type);
  if (i!=buildTime.end() && (i->second<=time || i->second==0)) return;
  buildTime[type]=time;
  if (time<0) return;
  for(std::map<const BWAPI::UnitType*,int>::const_iterator i=type.requiredUnits().begin();i!=type.requiredUnits().end();i++)
  {
    updateBuildTime(*i->first,time-i->first->buildTime());
  }
}

InformationManager::UnitData::UnitData()
{
  position=BWAPI::Positions::Unknown;
  type=BWAPI::UnitTypes::Unknown;
  player=NULL;
}