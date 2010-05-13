#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include <cmath>

static bool analyzed;
static bool analysis_just_finished;
static BWTA::Region* home;
static BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

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

  std::map<int, BWAPI::Unit*> ownUnits;
  std::map<int, BWAPI::Unit*> sightedEnemies;
  virtual void printPlayers();
  virtual BWAPI::Unit* getClosestUnit(BWAPI::Unit*);
  virtual void allUnitsAttackClosest();
  virtual BWAPI::Position getGroupCenter(std::map<int, BWAPI::Unit*>);
  virtual void unitEvade(BWAPI::Unit*);
  virtual BWAPI::Position getEvadePath(BWAPI::Unit*);
  virtual bool healthThreshold(BWAPI::Unit*);
  virtual BWAPI::Position calcEvadePath(int, int, BWAPI::Unit*);
  virtual void MoveToLine();
  virtual bool UnitIsFighter(BWAPI::Unit*);
  virtual BWAPI::Position getMapCenter();
  virtual std::pair<int, int> calculateMapCenter();
};