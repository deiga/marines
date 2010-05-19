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
  Broodwar->setLocalSpeed(25);
  BWTA::readMap();
  BWTA::analyze();
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
  this->buildOrderManager->buildAdditional(120,UnitTypes::Protoss_Zealot,70);
  this->buildOrderManager->buildAdditional(2,UnitTypes::Protoss_Gateway,60);

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
  //Broodwar->printf("Old: (%d, %d), New: (%d, %d)", Broodwar->self()->getStartLocation().x(), Broodwar->self()->getStartLocation().y(), newbase->getTilePosition().x(), newbase->getTilePosition().y());
  marines_log << Broodwar->getFrameCount() << ": Expand to (" << newbase->getTilePosition().x() << ", " << newbase->getTilePosition().y() << ")!" << endl;
	this->baseManager->expand(newbase, 50);
  this->buildManager->build(UnitTypes::Protoss_Pylon, newbase->getTilePosition());
  this->buildManager->build(UnitTypes::Protoss_Photon_Cannon, newbase->getTilePosition());
  this->buildManager->build(UnitTypes::Protoss_Photon_Cannon, newbase->getTilePosition());
  this->buildManager->build(UnitTypes::Protoss_Gateway, newbase->getTilePosition());
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

  if (Broodwar->getFrameCount() > 0 && Broodwar->getFrameCount() % 2000) {
    int zealot_count = Broodwar->self()->allUnitCount(UnitTypes::Protoss_Zealot);
	  if (zealot_count > 10) {
      expander();
    }
  }

  if (Broodwar->getFrameCount() > 0 && Broodwar->getFrameCount() % 8000 == 0) {
    marines_log << Broodwar->getFrameCount() << ": Dragoon queue! " << endl;
    //this->buildOrderManager->buildAdditional(80,UnitTypes::Protoss_Dragoon,60);
  }

  if (Broodwar->getFrameCount() > 0 && Broodwar->getFrameCount() % 3000 == 0 ) {
    int probe_count = Broodwar->self()->allUnitCount(UnitTypes::Protoss_Probe);
    if (probe_count > 50) {
		  this->workerManager->disableAutoBuild();
	  }
	  else {
		  this->workerManager->enableAutoBuild();
	  }
  }

  if (Broodwar->getFrameCount() > 0 && Broodwar->getFrameCount() % 1000 == 0)
    marines_log << Broodwar->getFrameCount() << ": Sending scout!" << endl;
    scoutManager->setScoutCount(1);

  set<Unit*> units=Broodwar->self()->getUnits();
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

int BasicAIModule::countUnits(UnitType type){
	int count = 0;
	set<Unit*> units=Broodwar->self()->getUnits(); 
	for (set<Unit*>::iterator u = units.begin(); u != units.end(); u++) {
		if ((*u)->getType() == type) {
			count++;
		}
	}
	return count;
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
}

void BasicAIModule::onUnitShow(BWAPI::Unit* unit)
{
  if (Broodwar->isReplay()) return;
  this->informationManager->onUnitShow(unit);
  this->unitGroupManager->onUnitShow(unit);
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
