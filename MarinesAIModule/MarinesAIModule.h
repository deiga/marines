#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include <cmath>

static bool analyzed;
static bool analysis_just_finished;
static BWTA::Region* home;
static BWTA::BaseLocation* homebase;
static BWTA::Region* enemy_base;
static BWAPI::TilePosition ownStartPosition;
static int buildDistance;
DWORD WINAPI AnalyzeThread();
static BWTA::RectangleArray<bool> reservedMap;

class MarinesAIModule : public BWAPI::AIModule
{
public:
  virtual void onStart();
  virtual void onEnd(bool);
  virtual void onFrame();
  virtual bool onSendText(std::string);
  virtual void onPlayerLeft(BWAPI::Player*);
  virtual void onUnitCreate(BWAPI::Unit*);
  virtual void onUnitDestroy(BWAPI::Unit*);
  virtual void onUnitShow(BWAPI::Unit*);
  virtual void onUnitHide(BWAPI::Unit*);
  /*virtual void onNukeDetect(BWAPI::Position);
  virtual void onUnitMorph(BWAPI::Unit*);
  virtual void onUnitRenegade(BWAPI::Unit*);*/
  virtual BWAPI::Unit* getClosestMineral(BWAPI::Unit*);
  void drawStats(); //not part of BWAPI::AIModule
  void showPlayers();
  void showForces();
  bool show_visibility_data;

  virtual BWAPI::TilePosition getBuildLocationNear(BWAPI::TilePosition, BWAPI::UnitType);
  virtual bool canBuildHereWithSpace(BWAPI::TilePosition, BWAPI::UnitType);	
  virtual bool canBuildHere(BWAPI::TilePosition, BWAPI::UnitType);
  virtual bool buildable(int, int);
  std::map<BWAPI::Unit*, std::pair<bool, int>> ownUnits;
  std::map<int, BWAPI::Unit*> sightedEnemies;
  std::vector<BWAPI::Unit*> mineral_probes;
  std::vector<BWAPI::Unit*> gas_probes;
  virtual void printPlayers();
  virtual BWAPI::Unit* getClosestUnit(BWAPI::Unit*);
  virtual BWAPI::Unit* getClosestOwnUnit(BWAPI::Unit*);
  virtual void allUnitsAttackClosest();
  virtual BWAPI::Position getGroupCenter(std::map<int, BWAPI::Unit*>);
  virtual BWAPI::Position getGroupCenter(std::map<BWAPI::Unit*, std::pair<bool, int>>);
  virtual void unitEvade(BWAPI::Unit*);
  virtual BWAPI::Position getEvadePath(BWAPI::Unit*);
  virtual bool healthThreshold(BWAPI::Unit*);
  virtual BWAPI::Position calcEvadePath(int, int, BWAPI::Unit*);
  virtual void MoveToLine();
  virtual bool UnitIsFighter(BWAPI::Unit*);
  virtual BWAPI::Position getMapCenter();
  virtual std::pair<int, int> calcMapCenter();
  virtual bool isFleeing(BWAPI::Unit*);
  virtual void evadeUnitsIfAttacked();
  virtual bool isAttacked(BWAPI::Unit*);
  virtual bool isZealot(BWAPI::Unit*);
};