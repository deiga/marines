#include "BasicAIModule.h"
#include "Util.h"
using namespace BWAPI;
using namespace std;

void BasicAIModule::onStart()
{
  marines_log.open("marines.log", fstream::out);
  this->showManagerAssignments=false;
  if (Broodwar->isReplay()) return;
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  //Broodwar->enableFlag(Flag::CompleteMapInformation);
  Broodwar->setLocalSpeed(15);
  BWTA::readMap();
  BWTA::analyze();
  expcounter = 0;
  this->analyzed=true;
  this->buildManager       = new BuildManager(&this->arbitrator);
  this->techManager        = new TechManager(&this->arbitrator);
  this->upgradeManager     = new UpgradeManager(&this->arbitrator);
  this->scoutManager       = new ScoutManager(&this->arbitrator);
  this->workerManager      = new WorkerManager(&this->arbitrator);
  this->supplyManager      = new SupplyManager();
  this->buildOrderManager  = new BuildOrderManager(this->buildManager,this->techManager,this->upgradeManager,this->workerManager,this->supplyManager);
  this->baseManager        = new BaseManager();
  this->defenseManager     = new DefenseManager(&this->arbitrator);
  this->informationManager = new InformationManager();
  this->borderManager      = new BorderManager();
  this->unitGroupManager   = new UnitGroupManager();
  this->enhancedUI         = new EnhancedUI();

  this->supplyManager->setBuildManager(this->buildManager);
  this->supplyManager->setBuildOrderManager(this->buildOrderManager);
  this->techManager->setBuildingPlacer(this->buildManager->getBuildingPlacer());
  this->upgradeManager->setBuildingPlacer(this->buildManager->getBuildingPlacer());
  this->workerManager->setBaseManager(this->baseManager);
  this->workerManager->setBuildOrderManager(this->buildOrderManager);
  this->baseManager->setBuildOrderManager(this->buildOrderManager);
  this->borderManager->setInformationManager(this->informationManager);
  this->baseManager->setBorderManager(this->borderManager);
  this->defenseManager->setBorderManager(this->borderManager);
  
  this->scoutManager->setDebugMode(true);
  BWAPI::Race race = Broodwar->self()->getRace();
  BWAPI::Race enemyRace = Broodwar->enemy()->getRace();
  BWAPI::UnitType workerType=*(race.getWorker());
  double minDist;
  BWTA::BaseLocation* natural=NULL;
  BWTA::BaseLocation* home=BWTA::getStartLocation(Broodwar->self());
  for(set<BWTA::BaseLocation*>::const_iterator b=BWTA::getBaseLocations().begin();b!=BWTA::getBaseLocations().end();b++)
  {
    if (*b==home) continue;
    double dist=home->getGroundDistance(*b);
    if (dist>0)
    {
      if (natural==NULL || dist<minDist)
      {
        minDist=dist;
        natural=*b;
      }
    }
  }
  this->buildOrderManager->enableDependencyResolver();
  //make the basic production facility
  
  this->buildOrderManager->buildAdditional(1, UnitTypes::Protoss_Forge, 60);
  this->buildOrderManager->buildAdditional(45,UnitTypes::Protoss_Zealot,70);
  this->buildOrderManager->buildAdditional(1,UnitTypes::Protoss_Gateway,60);
  this->buildOrderManager->buildAdditional(1,UnitTypes::Protoss_Gateway,40);
  this->buildOrderManager->buildAdditional(1,UnitTypes::Protoss_Gateway,20);

  this->workerManager->enableAutoBuild();
  this->workerManager->setAutoBuildPriority(90);

  marines_log << "onStart() done!" << endl;
}

BasicAIModule::~BasicAIModule()
{
  delete this->buildManager;
  delete this->techManager;
  delete this->upgradeManager;
  delete this->scoutManager;
  delete this->workerManager;
  delete this->supplyManager;
  delete this->buildOrderManager;
  delete this->baseManager;
  delete this->defenseManager;
  delete this->informationManager;
  delete this->borderManager;
  delete this->unitGroupManager;
  delete this->enhancedUI;
}
void BasicAIModule::onEnd(bool isWinner)
{
  log("onEnd(%d)\n",isWinner);
}

void BasicAIModule::expander() {
  BWTA::BaseLocation* newbase = &getNearestExpansion();
  if (newbase == NULL) {
    return;
  }
  marines_log << Broodwar->getFrameCount() << ": Expand to (" << newbase->getTilePosition().x() << ", " << newbase->getTilePosition().y() << ")!" << endl;
  //this->buildOrderManager->buildAdditional(1, UnitTypes::Protoss_Nexus, 100, newbase->getTilePosition());
  //this->baseManager->expand(newbase, 90);B
  this->buildOrderManager->buildAdditional(2, UnitTypes::Protoss_Photon_Cannon, 70, newbase->getTilePosition());
  this->buildOrderManager->buildAdditional(1, UnitTypes::Protoss_Gateway, 70, newbase->getTilePosition());
}

void BasicAIModule::onFrame()
{
  if (Broodwar->isReplay()) return;
  if (!this->analyzed) return;
  this->buildManager->update();
  this->buildOrderManager->update();
  this->baseManager->update();
  this->workerManager->update();
  this->techManager->update();
  this->upgradeManager->update();
  this->supplyManager->update();
  this->scoutManager->update();
  this->enhancedUI->update();
  this->borderManager->update();
  this->defenseManager->update();
  this->arbitrator.update();
  drawStats();
 // Ala: 2016, 3744 Yla: 2342, 500
  if (Broodwar->getFrameCount() == 10) {
	  Broodwar->printf("X: %d, Y: %d",  Broodwar->self()->getStartLocation().x()*32, Broodwar->self()->getStartLocation().y()*32);
	  //Broodwar->printf("Pylon buildtime: %d", BWAPI::UnitTypes::Protoss_Pylon.buildTime());
  }

  if (Broodwar->getFrameCount() > 0 && Broodwar->getFrameCount() % 600 == 0) {

    int zealot_count = Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Zealot);
    marines_log << Broodwar->getFrameCount() << ": Planning to Expand! " << endl;
	  if (zealot_count > 10 && Broodwar->self()->minerals() >= 200 && expcounter == 0) {
		  Broodwar->printf("Pylonia tekee");
      marines_log << Broodwar->getFrameCount() << ": Pylonia tekee " << endl;
		  BWTA::BaseLocation* newbase = &getNearestExpansion();
		  this->buildOrderManager->buildAdditional(1, UnitTypes::Protoss_Pylon, 100, BWAPI::TilePosition(newbase->getTilePosition().x()-3, newbase->getTilePosition().y()-2));
      this->baseManager->expand(newbase);
		  this->borderManager->addMyBase(newbase);
		  expcounter++;
	  } else if (zealot_count > 10 && Broodwar->self()->minerals() >= 600) {
      marines_log << Broodwar->getFrameCount() << ": Puuhataan base " << endl;
		  Broodwar->printf("Nyt puuhataan base");
      expander();
		  expcounter = 0;
    }
  }

  if (Broodwar->getFrameCount() % 24 == 0) {
    marines_log << Broodwar->getFrameCount() << ": Selecting all enemies " << endl;
    sightedEnemies = SelectAllEnemy();
    allUnitsAttackClosest();
  }

  if (Broodwar->getFrameCount() % 100 == 0) {

	  if (Broodwar->getFrameCount() < 6000 && Broodwar->self()->allUnitCount(UnitTypes::Protoss_Zealot) >= 15) {
      Broodwar->sendText("Attaaack! ...of the killer tomatoes!");
		  for (std::set<Unit*>::iterator i=units.begin();i!=units.end();i++) {
			  if ((*i)->getType() == UnitTypes::Protoss_Zealot || (*i)->getType() == UnitTypes::Protoss_Dragoon) {
				  if (!enemyBuildings.empty()){
				  Broodwar->drawLine(CoordinateType::Map, (*i)->getPosition().x(), (*i)->getPosition().y(), (*enemyBuildings.begin()).x(), (*enemyBuildings.begin()).y(), Colors::Green);
					  (*i)->attackMove((*enemyBuildings.begin()));
				  }
			  }
		  }
	  }
	  else if ((Broodwar->self()->allUnitCount(UnitTypes::Protoss_Zealot) > 25 && Broodwar->self()->allUnitCount(UnitTypes::Protoss_Dragoon) > 10) || Broodwar->self()->supplyUsed() > 320) {
      Broodwar->printf("Hep");
		  std::set<Unit*> units=Broodwar->self()->getUnits();
		  for (std::set<Unit*>::iterator i=units.begin();i!=units.end();i++) {
			  if ((*i)->getType() == UnitTypes::Protoss_Zealot || (*i)->getType() == UnitTypes::Protoss_Dragoon) {
				  if (!enemyBuildings.empty()){
				  Broodwar->drawLine(CoordinateType::Map, (*i)->getPosition().x(), (*i)->getPosition().y(), (*enemyBuildings.begin()).x(), (*enemyBuildings.begin()).y(), Colors::Green);
					  (*i)->attackMove((*enemyBuildings.begin()));
				  }
			  }
		  }
	  }
  }

  if (Broodwar->getFrameCount() > 0 && (Broodwar->getFrameCount() % 10000 == 0 || Broodwar->self()->allUnitCount(UnitTypes::Protoss_Zealot) > 30)) {
    marines_log << Broodwar->getFrameCount() << ": Dragoon queue! " << endl;
    this->buildOrderManager->build(2, UnitTypes::Protoss_Assimilator, 80);
    this->upgradeManager->upgrade(UpgradeTypes::Singularity_Charge);
    this->upgradeManager->upgrade(UpgradeTypes::Protoss_Armor);
    this->upgradeManager->upgrade(UpgradeTypes::Protoss_Ground_Weapons);
    this->upgradeManager->upgrade(UpgradeTypes::Protoss_Plasma_Shields);
    this->buildOrderManager->build(40,UnitTypes::Protoss_Dragoon,60);
    this->buildOrderManager->build(40,UnitTypes::Protoss_Zealot,60);
  }

  if (Broodwar->getFrameCount() > 0 && Broodwar->getFrameCount() % 2000 == 0 ) {
    int probe_count = Broodwar->self()->allUnitCount(UnitTypes::Protoss_Probe);
    if (probe_count >= 40) {
      marines_log << Broodwar->getFrameCount() << ": Stop probe building. " << endl;
		  this->workerManager->disableAutoBuild();
	  }
	  else if (probe_count < 35) {
      marines_log << Broodwar->getFrameCount() << ": Re-enable probe building! " << endl;
		  this->workerManager->enableAutoBuild();
      this->workerManager->setAutoBuildPriority(40);
	  }
  }

  if (Broodwar->getFrameCount() > 0 && Broodwar->getFrameCount() % 1000 == 0) {
    marines_log << Broodwar->getFrameCount() << ": Sending scout!" << endl;
    if (scoutManager->isScouting()) {
      if (this->scoutManager->scouts.begin()->second.mode == ScoutManager::ScoutData::Idle) {
        this->scoutManager->setScoutCount(0);
        delete this->scoutManager;
        this->scoutManager = new ScoutManager(&this->arbitrator);
        this->scoutManager->setDebugMode(true);
        this->scoutManager->setScoutCount(1);
      }
    } else {
      this->scoutManager->setScoutCount(1);
    }
  }
  
  if (Broodwar->getFrameCount() % 24 == 0) {
    units = Broodwar->self()->getUnits();
  }
  if (this->showManagerAssignments)
  {
    for(set<Unit*>::iterator i=units.begin();i!=units.end();i++)
    {
      if (this->arbitrator.hasBid(*i))
      {
        int x=(*i)->getPosition().x();
        int y=(*i)->getPosition().y();
        list< pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> > bids=this->arbitrator.getAllBidders(*i);
        int y_off=0;
        bool first = false;
        const char activeColor = '\x07', inactiveColor = '\x16';
        char color = activeColor;
        for(list< pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> >::iterator j=bids.begin();j!=bids.end();j++)
        {
          Broodwar->drawTextMap(x,y+y_off,"%c%s: %d",color,j->first->getShortName().c_str(),(int)j->second);
          y_off+=15;
          color = inactiveColor;
        }
      }
    }
  }

  

  UnitGroup myPylonsAndGateways = SelectAll()(Pylon,Gateway)(HitPoints,"<=",200);
  for each(Unit* u in myPylonsAndGateways)
  {
    Broodwar->drawCircleMap(u->getPosition().x(),u->getPosition().y(),20,Colors::Red);
  }
}

BWTA::BaseLocation& BasicAIModule::getNearestExpansion(){
  marines_log << Broodwar->getFrameCount() << ": getNearestExpansion()" << endl;
	set<BWTA::BaseLocation*> pesat = BWTA::getBaseLocations();
	pair<double, BWTA::BaseLocation*> distance = pair<double, BWTA::BaseLocation*>(99999.0, NULL);
	for(set<BWTA::BaseLocation*>::const_iterator b = pesat.begin(); b != pesat.end(); b++){
		if (this->baseManager->getBase((*b)) == NULL) {
			double temp_dist = (*b)->getGroundDistance(BWTA::getStartLocation(Broodwar->self()));
			if((temp_dist) < distance.first) {
				distance = pair<double, BWTA::BaseLocation*>(temp_dist, (*b));
			}
		}
	}
	return *distance.second;
}

void BasicAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
  if (Broodwar->isReplay()) return;
  this->arbitrator.onRemoveObject(unit);
  this->buildManager->onRemoveUnit(unit);
  this->techManager->onRemoveUnit(unit);
  this->upgradeManager->onRemoveUnit(unit);
  this->workerManager->onRemoveUnit(unit);
  this->scoutManager->onRemoveUnit(unit);
  this->defenseManager->onRemoveUnit(unit);
  this->informationManager->onUnitDestroy(unit);
  this->baseManager->onRemoveUnit(unit);
  if (!enemyBuildings.empty()){
    enemyBuildings.erase(unit->getPosition());
  }
  if (unit->getType() == UnitTypes::Protoss_Zealot) {
    this->buildOrderManager->buildAdditional(2,UnitTypes::Protoss_Zealot,55);
  }
  else if (unit->getType() == UnitTypes::Protoss_Dragoon) {
    this->buildOrderManager->buildAdditional(1,UnitTypes::Protoss_Dragoon,40);
  }
}

void BasicAIModule::onUnitCreate(Unit* unit) {
  if (!Broodwar->isReplay()) {
 
  } else {
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


void BasicAIModule::onUnitShow(BWAPI::Unit* unit)
{
  if (Broodwar->isReplay()) return;
  this->informationManager->onUnitShow(unit);
  this->unitGroupManager->onUnitShow(unit);
  if (unit->getType().isBuilding() && unit->getPlayer()->isEnemy(Broodwar->self())) {
	  enemyBuildings.insert(unit->getPosition());
	  Broodwar->printf("Enemy building: X: %d, Y: %d", unit->getPosition().x(), unit->getPosition().y());
    if (sightedEnemies.size() == 1) {
      UnitGroup ownZealots = SelectAll(UnitTypes::Protoss_Zealot);
      for(UnitGroup::iterator it = ownZealots.begin(); it != ownZealots.end(); it++) {
        (*it)->attackMove(unit->getPosition());
      } 
    }
  }
}
void BasicAIModule::onUnitHide(BWAPI::Unit* unit)
{
  if (Broodwar->isReplay()) return;
  this->informationManager->onUnitHide(unit);
  this->unitGroupManager->onUnitHide(unit);
}

void BasicAIModule::onUnitMorph(BWAPI::Unit* unit)
{
  if (Broodwar->isReplay()) return;
  this->unitGroupManager->onUnitMorph(unit);
}
void BasicAIModule::onUnitRenegade(BWAPI::Unit* unit)
{
  if (Broodwar->isReplay()) return;
  this->unitGroupManager->onUnitRenegade(unit);
}

bool BasicAIModule::onSendText(string text)
{
  if (Broodwar->isReplay()) return true;
  UnitType type=UnitTypes::getUnitType(text);
  if (text=="debug")
  {
    if (this->showManagerAssignments==false)
    {
      this->showManagerAssignments=true;
      this->buildOrderManager->setDebugMode(true);
      this->scoutManager->setDebugMode(true);
    }
    else
    {
      this->showManagerAssignments=false;
      this->buildOrderManager->setDebugMode(false);
      this->scoutManager->setDebugMode(false);
    }
    return true;
  }
  if (text=="expand")
  {
    this->baseManager->expand();
  }
  if (type!=UnitTypes::Unknown)
  {
    this->buildOrderManager->buildAdditional(1,type,300);
  }
  else
  {
    TechType type=TechTypes::getTechType(text);
    if (type!=TechTypes::Unknown)
    {
      this->techManager->research(type);
    }
    else
    {
      UpgradeType type=UpgradeTypes::getUpgradeType(text);
      if (type!=UpgradeTypes::Unknown)
      {
        this->upgradeManager->upgrade(type);
      }
      else
        Broodwar->printf("You typed '%s'!",text.c_str());
    }
  }
  return true;
}

void BasicAIModule::drawStats() {
  set<Unit*> myUnits = Broodwar->self()->getUnits();
  Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
  map<UnitType, int> unitTypeCounts;
  for(set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++) {
    if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end()) {
      unitTypeCounts.insert(make_pair((*i)->getType(),0));
    }
    unitTypeCounts.find((*i)->getType())->second++;
  }
  int line=1;
  for(map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++) {
    Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
    line++;
  }
}

Unit* BasicAIModule::getClosestUnit(Unit* unit) {
  pair<double, Unit*> minDist = make_pair(9999999.0, unit);

  for(UnitGroup::const_iterator it = sightedEnemies.begin(); it != sightedEnemies.end(); it++) {
    if ((*it)->getDistance(unit) < minDist.first) {
      minDist = make_pair((*it)->getDistance(unit), (*it));
    }
  }
  //Broodwar->printf("Player: %s", minDist.second->getPlayer()->getName().c_str());
  return minDist.second;
}

void BasicAIModule::allUnitsAttackClosest() {
  UnitGroup own_units = SelectAll();
  for(UnitGroup::const_iterator it = own_units.begin(); it != own_units.end(); it++) {
    if (!sightedEnemies.empty()) {
      Unit* target = getClosestUnit((*it));
      Unit* own = (*it);
      own->attackUnit(target);
      Broodwar->drawLine(CoordinateType::Map, own->getPosition().x(), own->getPosition().y(), target->getPosition().x(), target->getPosition().y(), Colors::Red);
    }
  }
}
