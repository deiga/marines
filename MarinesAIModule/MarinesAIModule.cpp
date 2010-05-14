#include "MarinesAIModule.h"
using namespace BWAPI;

void MarinesAIModule::onStart() {
  Broodwar->setLocalSpeed(60);
  Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  // Uncomment to enable complete map information
  //Broodwar->enableFlag(Flag::CompleteMapInformation);

  //read map information into BWTA so terrain analysis can be done in another thread
  BWTA::readMap();
  analyzed=false;
  analysis_just_finished=false;
  show_visibility_data=false;

  printPlayers();
  if ( !Broodwar->isReplay() ) {
    sightedEnemies.clear();
    for(std::set<Unit*>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++) {
      //send each worker to the mineral field that is closest to it
      if ( (*i)->getType().isWorker( )) {
        Unit* closestMineral = getClosestMineral(*i);
        if ( closestMineral != NULL ) {
          (*i)->rightClick(closestMineral);
        }
      } else if ( (*i)->getType().isResourceDepot() ) {
        //if this is a center, tell it to build the appropiate type of worker
        if ( (*i)->getType().getRace() != Races::Zerg ) {
          (*i)->train(*Broodwar->self()->getRace().getWorker());
        }
      } else if ( UnitIsFighter(*i) ) {
        ownUnits.insert(std::pair<Unit*, std::pair<bool, int>>((*i), std::pair<bool, int>(false, (*i)->getHitPoints() )));
      }
    }
    //Position temp = getGroupCenter(ownUnits);
    //Broodwar->printf("startX: %d, startY: %d", temp.x(), temp.y());
    /* for(std::map<int, Unit*>::const_iterator it = ownUnits.begin(); it != ownUnits.end(); it++) {
    (*it).second->rightClick(Position(800, 1173));
    }*/
    MoveToLine();
  }
}

void MarinesAIModule::onEnd(bool isWinner) {
  if ( isWinner ) {
    //log win to file
  }
}

void MarinesAIModule::onFrame() {
  if (show_visibility_data) {    
    for(int x=0;x<Broodwar->mapWidth();x++) {
      for(int y=0;y<Broodwar->mapHeight();y++) {
        if (Broodwar->isExplored(x,y)) {
          if (Broodwar->isVisible(x,y)) {
            Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Green);
          }
          else {
            Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Blue);
          }
        }
        else {
          Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Red);
        }
      }
    }
  }

  if (Broodwar->isReplay()) {
    return;
  }
  else {
    if (Broodwar->getFrameCount() % 30 == 0) {
      //allUnitsAttackClosest();
    }
    if (Broodwar->getFrameCount() % 10 == 0) {
      evadeUnitsIfAttacked();
    }
    /*if (!sightedEnemies.empty()) {
      for (std::map<Unit*, std::pair<bool, int>>::const_iterator it = ownUnits.begin(); it != ownUnits.end(); it++) {
        if ( healthThreshold((*it).first) ) {
          unitEvade((*it).first);
        }
      }
    }*/
  }

  drawStats();
  if (analyzed && Broodwar->getFrameCount()%30==0) {
    //order one of our workers to guard our chokepoint.
    for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++) {
      if ((*i)->getType().isWorker()) {
        //get the chokepoints linked to our home region
        std::set<BWTA::Chokepoint*> chokepoints = home->getChokepoints();
        double min_length = 10000;
        BWTA::Chokepoint* choke = NULL;

        //iterate through all chokepoints and look for the one with the smallest gap (least width)
        for(std::set<BWTA::Chokepoint*>::iterator c = chokepoints.begin(); c != chokepoints.end(); c++) {
          double length = (*c)->getWidth();
          if ( length < min_length || choke == NULL ) {
            min_length = length;
            choke = *c;
          }
        }

        //order the worker to move to the center of the gap
        (*i)->rightClick(choke->getCenter());
        break;
      }
    }
  }

  if (analyzed) {
    //we will iterate through all the base locations, and draw their outlines.
    for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++) {
      TilePosition p=(*i)->getTilePosition();
      Position c=(*i)->getPosition();

      //draw outline of center location
      Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

      //draw a circle at each mineral patch
      for(std::set<Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();j++) {
        Position q=(*j)->getInitialPosition();
        Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
      }

      //draw the outlines of vespene geysers
      for(std::set<Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++) {
        TilePosition q=(*j)->getInitialTilePosition();
        Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
      }

      //if this is an island expansion, draw a yellow circle around the base location
      if ((*i)->isIsland()) {
        Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
      }
    }

    //we will iterate through all the regions and draw the polygon outline of it in green.
    for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++) {
      BWTA::Polygon p=(*r)->getPolygon();
      for(int j=0;j<(int)p.size();j++) {
        Position point1=p[j];
        Position point2=p[(j+1) % p.size()];
        Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
      }
    }

    //we will visualize the chokepoints with red lines
    for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++) {
      for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++) {
        Position point1=(*c)->getSides().first;
        Position point2=(*c)->getSides().second;
        Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
      }
    }
  }
  if (analysis_just_finished) {
    Broodwar->printf("Finished analyzing map.");
    analysis_just_finished=false;
  }
}

void MarinesAIModule::onUnitCreate(Unit* unit) {
  if (!Broodwar->isReplay()) {
    Broodwar->sendText("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  }
  else{
    /*if we are in a replay, then we will print out the build order
    (just of the buildings, not the units).*/
    if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false) {
      int seconds=Broodwar->getFrameCount()/24;
      int minutes=seconds/60;
      seconds%=60;
      Broodwar->sendText("%.2d:%.2d: %s creates a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
    }
  }
}

void MarinesAIModule::onUnitDestroy(Unit* unit) {
  if (!Broodwar->isReplay()) {
    //Broodwar->sendText("A %s [%x] has been destroyed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
    if (unit->getPlayer()->isEnemy(Broodwar->self())) {
      sightedEnemies.erase(unit->getID());
      //allUnitsAttackClosest();
    } else if (unit->getPlayer() == Broodwar->self()) {
      ownUnits.erase(unit);
    }
  }
}

void MarinesAIModule::onUnitShow(Unit* unit) {
  if (!Broodwar->isReplay() && unit->getPlayer()->isEnemy(Broodwar->self())) {
    //Broodwar->printf("UnitType: %s", unit->getType().getName().c_str());
    sightedEnemies.insert(std::pair<int, Unit*>(unit->getID(), unit));
    if (sightedEnemies.size() == 1) {
      for(std::map<Unit*, std::pair<bool, int>>::const_iterator it = ownUnits.begin(); it != ownUnits.end(); it++) {
        (*it).first->attackUnit(unit);
      } 
    }
  }
}

void MarinesAIModule::onUnitHide(Unit* unit) {
  //  if (!Broodwar->isReplay())
  //  Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  if (unit->getPlayer()->isEnemy(Broodwar->self())) {
    sightedEnemies.erase(unit->getID());
  }
}

void MarinesAIModule::onPlayerLeft(Player* player) {
  Broodwar->sendText("%s left the game.",player->getName().c_str());
}

/*
void MarinesAIModule::onNukeDetect(Position target) {
if (target!=Positions::Unknown) {
Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
}
else {
Broodwar->printf("Nuclear Launch Detected");
}
}
void MarinesAIModule::onUnitRenegade(Unit* unit) {
if (!Broodwar->isReplay()) {
Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}
}

void MarinesAIModule::onUnitMorph(Unit* unit) {
if (!Broodwar->isReplay()) {
Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}
else {
//if we are in a replay, then we will print out the build order (just of the buildings, not the units).
if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false) {
int seconds=Broodwar->getFrameCount()/24;
int minutes=seconds/60;
seconds%=60;
Broodwar->sendText("%.2d:%.2d: %s morphs a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
}
}
}
*/

bool MarinesAIModule::onSendText(std::string text) {
  if (text=="/show players") {
    showPlayers();
    return false;
  } else if (text=="/show forces") {
    showForces();
    return false;
  } else if (text=="/show visibility") {
    show_visibility_data=true;
  } else if (text=="/analyze") {
    if (analyzed == false) {
      Broodwar->printf("Analyzing map... this may take a minute");
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
    }
    return false;
  } else {
    Broodwar->printf("You typed '%s'!",text.c_str());
  }
  return true;
}

// ---------- Example functions ----------

DWORD WINAPI AnalyzeThread() {
  BWTA::analyze();
  analyzed   = true;
  analysis_just_finished = true;

  //self start location only available if the map has base locations
  if (BWTA::getStartLocation(Broodwar->self())!=NULL) {
    home = BWTA::getStartLocation(Broodwar->self())->getRegion();
  }
  //enemy start location only available if Complete Map Information is enabled.
  if (BWTA::getStartLocation(Broodwar->enemy())!=NULL) {
    enemy_base = BWTA::getStartLocation(Broodwar->enemy())->getRegion();
  }
  return 0;
}

void MarinesAIModule::drawStats() {
  std::set<Unit*> myUnits = Broodwar->self()->getUnits();
  Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
  std::map<UnitType, int> unitTypeCounts;
  for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++) {
    if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end()) {
      unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
    }
    unitTypeCounts.find((*i)->getType())->second++;
  }
  int line=1;
  for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++) {
    Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
    line++;
  }
}

void MarinesAIModule::showPlayers() {
  std::set<Player*> players=Broodwar->getPlayers();
  for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++) {
    Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
  }
}

void MarinesAIModule::showForces() {
  std::set<Force*> forces=Broodwar->getForces();
  for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++) {
    std::set<Player*> players=(*i)->getPlayers();
    Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
    for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++) {
      Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
    }
  }
}

void MarinesAIModule::printPlayers() {
  if (Broodwar->isReplay()) {
    Broodwar->printf("The following players are in this replay:");
    for(std::set<Player*>::iterator pIt = Broodwar->getPlayers().begin(); pIt != Broodwar->getPlayers().end(); pIt++) {
      if (!(*pIt)->getUnits().empty() && !(*pIt)->isNeutral()) {
        Broodwar->printf("%s, playing as a %s",(*pIt)->getName().c_str(),(*pIt)->getRace().getName().c_str());
      }
    }
  } else {
    Broodwar->printf("The match up is %s vs %s",
      Broodwar->self()->getRace().getName().c_str(),
      Broodwar->enemy()->getRace().getName().c_str());
  }
}

Unit* MarinesAIModule::getClosestMineral(Unit* unit) {
  Unit* closestMineral = NULL;
  for(std::set<Unit*>::iterator mIt = Broodwar->getMinerals().begin(); mIt != Broodwar->getMinerals().end(); mIt++) {
    if ( closestMineral == NULL || unit->getDistance(*mIt) < unit->getDistance(closestMineral) ) {
      closestMineral = *mIt;
    }
  }
  return closestMineral;
}

// ---------- Own functions ----------

void MarinesAIModule::MoveToLine() {
  Position mapCenter = getMapCenter();
  Position groupCenter = getGroupCenter(ownUnits);
  int diffX = groupCenter.x() - mapCenter.x();
  int xCoord = (int)(diffX * 0.75) * (-1) + groupCenter.x();
  int y = 800;
  Broodwar->printf("Target pos: x: %d, y: %d", xCoord, y);  
  for(std::map<Unit*, std::pair<bool, int>>::const_iterator it = ownUnits.begin(); it != ownUnits.end(); it++) {
    y += 50;
    (*it).first->rightClick(Position(xCoord, y));
  }
}

void MarinesAIModule::allUnitsAttackClosest() {
  for(std::map<Unit*, std::pair<bool, int>>::const_iterator it = ownUnits.begin(); it != ownUnits.end(); it++) {
    if (!sightedEnemies.empty()) {
      (*it).first->attackUnit(getClosestUnit((*it).first));
    }
  }
}

Position MarinesAIModule::getGroupCenter(std::map<int, Unit*> unitGroup) {
  if (unitGroup.empty()) {
    return Positions::None;
  }
  if (unitGroup.size() == 1) {
    return (unitGroup.begin()->second->getPosition());
  }
  int count, x, y;
  count = x = y = 0;
  for(std::map<int,Unit*>::const_iterator i = unitGroup.begin(); i != unitGroup.end(); i++) {
    Position p((*i).second->getPosition());
    if ( p != Positions::None && p != Positions::Unknown && p != Positions::Invalid ) {
      count++;
      x += p.x();
      y += p.y();
    }
  }
  if (count == 0) {
    return Positions::None;
  }

  Position temp = Position( x / count, y / count );
  return temp;
}

Position MarinesAIModule::getGroupCenter(std::map<Unit*, std::pair<bool, int>> unitGroup) {
  if (unitGroup.empty()) {
    return Positions::None;
  }
  if (unitGroup.size() == 1) {
    return (unitGroup.begin()->first->getPosition());
  }
  int count, x, y;
  count = x = y = 0;
  for(std::map<Unit*, std::pair<bool, int>>::const_iterator i = unitGroup.begin(); i != unitGroup.end(); i++) {
    Position p((*i).first->getPosition());
    if ( p != Positions::None && p != Positions::Unknown && p != Positions::Invalid ) {
      count++;
      x += p.x();
      y += p.y();
    }
  }
  if (count == 0) {
    return Positions::None;
  }

  Position temp = Position( x / count, y / count );
  return temp;
}

void MarinesAIModule::unitEvade(Unit* unit) {
  Position path = getEvadePath(unit);
  unit->rightClick(path);
  ownUnits[unit].first = true;
  Broodwar->drawLine(CoordinateType::Map,unit->getPosition().x(),unit->getPosition().y(),path.x(),path.y(),Colors::Green);
}

bool MarinesAIModule::isFleeing(Unit* unit) {
  return ownUnits[unit].first;
}

Position MarinesAIModule::calcEvadePath(int xDir, int yDir, Unit* unit) {
  int xCoord, yCoord;
  if ( unit->getType() == UnitTypes::Protoss_Dragoon ) {
    xCoord = (int)(xDir / 3.5) * (-1) + unit->getPosition().x();
    yCoord = (int)(yDir / 3.5) * (-1) + unit->getPosition().y();
  } else if ( unit->getType() == UnitTypes::Protoss_Zealot ) {

  }
  return Position(xCoord, yCoord);
}

Position MarinesAIModule::getEvadePath(Unit* unit) {
  Position enemyPos = getGroupCenter(sightedEnemies);
  Position unitPos = unit->getPosition();
  int diffX = enemyPos.x() - unitPos.x();
  int diffY = enemyPos.y() - unitPos.y();
  return calcEvadePath(diffX, diffY, unit);
}

Unit* MarinesAIModule::getClosestUnit(Unit* unit) {
  std::pair<double, Unit*> minDist = std::pair<double, Unit*>(9999999.0, NULL);

  for(std::map<int, Unit*>::const_iterator it = sightedEnemies.begin(); it != sightedEnemies.end(); it++) {
    if ((*it).second->getDistance(unit) < minDist.first) {
      minDist = std::pair<double, Unit*>((*it).second->getDistance(unit), (*it).second);
    }
  }
  //Broodwar->printf("Player: %s", minDist.second->getPlayer()->getName().c_str());
  return minDist.second;
}

bool MarinesAIModule::healthThreshold(Unit* target) {
  if ( target->getShields() == 0 ) {
    return true;
  } else if ( target->getHitPoints() == target->getType().maxHitPoints()*0.75 ) {
    return true;
  } else if ( target->getHitPoints() == target->getType().maxHitPoints()*0.50 ) {
    return true;
  } else if ( target->getHitPoints() == target->getType().maxHitPoints()*0.35 ) {
    return true;
  } else return false;
}

bool MarinesAIModule::UnitIsFighter(Unit* unit) {
  return !unit->getType().isBuilding() && unit->getType().canAttack();
}

Position MarinesAIModule::getMapCenter() {
  std::pair<int, int> temp = calcMapCenter();
  return Position(temp.first*TILE_SIZE, temp.second*TILE_SIZE);
}

std::pair<int, int> MarinesAIModule::calcMapCenter() {
  std::pair<int, int> center_coords = std::pair<int, int>(0, 0);
  int height, width;
  height = Broodwar->mapHeight();
  width = Broodwar->mapWidth();
  center_coords = std::pair<int, int>(height / 2, width / 2);
  return center_coords;
}

void MarinesAIModule::evadeUnitsIfAttacked() {
  for(std::map<Unit*, std::pair<bool, int>>::iterator uIt = ownUnits.begin(); uIt != ownUnits.end(); uIt++) {
    Unit* currUnit = (*uIt).first;
    if (isAttacked(currUnit)) {
      unitEvade(currUnit);
    }
    ownUnits[currUnit].second = currUnit->getHitPoints();
  }
}

bool MarinesAIModule::isAttacked(Unit* unit) {
  int lastHP = ownUnits[unit].second;
  int currHP = unit->getHitPoints();
  return (int)(lastHP * 0.8) >= currHP;
}