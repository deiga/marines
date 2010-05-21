#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <Arbitrator.h>
#include <WorkerManager.h>
#include <SupplyManager.h>
#include <BuildManager.h>
#include <BuildOrderManager.h>
#include <TechManager.h>
#include <UpgradeManager.h>
#include <BaseManager.h>
#include <ScoutManager.h>
#include <DefenseManager.h>
#include <InformationManager.h>
#include <BorderManager.h>
#include <UnitGroupManager.h>
#include <EnhancedUI.h>
#include <fstream>

class BasicAIModule : public BWAPI::AIModule
{
public:
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onUnitShow(BWAPI::Unit* unit);
  virtual void onUnitHide(BWAPI::Unit* unit);
  virtual void onUnitMorph(BWAPI::Unit* unit);
  virtual void onUnitRenegade(BWAPI::Unit* unit);
  virtual void onUnitDestroy(BWAPI::Unit* unit);
  virtual void onUnitCreate(BWAPI::Unit*);
  virtual bool onSendText(std::string text);
  ~BasicAIModule(); //not part of BWAPI::AIModule
  void drawStats(); //not part of BWAPI::AIModule
  void showPlayers();
  void showForces();
  bool analyzed;

  void expander();

  std::fstream marines_log;
  std::set<BWAPI::Unit*> units;

  std::set<BWAPI::Position> enemyBuildings;
  virtual BWTA::BaseLocation& getNearestExpansion();
  std::map<BWAPI::Unit*,BWAPI::UnitType> buildings;
  Arbitrator::Arbitrator<BWAPI::Unit*,double> arbitrator;
  WorkerManager* workerManager;
  SupplyManager* supplyManager;
  BuildManager* buildManager;
  TechManager* techManager;
  UpgradeManager* upgradeManager;
  BaseManager* baseManager;
  ScoutManager* scoutManager;
  BuildOrderManager* buildOrderManager;
  DefenseManager* defenseManager;
  InformationManager* informationManager;
  BorderManager* borderManager;
  UnitGroupManager* unitGroupManager;
  EnhancedUI* enhancedUI;
  bool showManagerAssignments;
  int expcounter;
};