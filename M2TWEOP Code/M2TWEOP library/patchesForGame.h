#pragma once
#include "army.h"
#include "stratModelsChange.h"
#include "graphicsD3D.h"
#include "cultures.h"
#include "faction.h"
#include "gameHelpers.h"
#include "unit.h"

struct aiPersonalityValues;
struct aiLongTermGoalDirector;
struct unitRQ;
struct bannerData;
struct aiRegionData;
struct aiRegionController;
struct settlementStats;
struct settlementStatsManager;
struct aiCampaignController;
struct aiGlobalStrategyDirector;
struct buildingsQueue;
struct buildingInQueue;
struct portBuildingStruct;
struct aiProductionController;
struct regionStruct;
struct aiFaction;
struct buildingLevel;
struct edbEntry;
struct settlementStruct;
struct stringWithHash;
struct unitDb;
class m_IDirect3DTexture9;

struct trackedArmy
{
	DWORD vfTable;
	armyStruct* army;
};

struct generalAssault
{
	DWORD vtbl;
	int8_t finished : 1;
	int8_t started : 1;
	int8_t init : 1;
	int8_t stop : 1;
	character* character;
	int xCoordStart;
	int yCoordStart;
	int xCoordTarget;
	int yCoordTarget;
	settlementStruct* settlement;
	bool nightBattle;
	char pad[3];
};

struct fakeFormSquareVtbl
{
	char pad[0x4C];
	int specPosGetFunc;
	char pad2[0xF4];
};

struct formationGame
{
	fakeFormSquareVtbl* vtbl; 
};

struct formationData
{
	float offsetRowX;
	float offsetRowY;
	float offsetColumnX;
	float offsetColumnY;
	float spacingRows;
	float spacingColumns;
	float angle;
	float entryXRadius;
	float entryYRadius;
	unsigned __int16 entries;
	unsigned __int16 width;
	unsigned __int8 specEntries;
	unsigned __int8 remainder;
	unsigned __int8 training;
	unsigned __int8 allowRandomPerturbations;
	void *auxiliaryData;
	int movingThrough;
};


class patchesForGame
{
public:
	//select worldpkgdesc from db
	static worldRecord* __fastcall selectWorldpkgdesc(char* database, worldRecord* selectedRecord);

	
	static void __fastcall onLoadSettlementWorldpkgdesc(worldRecord* selectedRecord);
	static int __fastcall onFortificationLevelS(settlementStruct* settlement, bool* isCastle);
	static char* __fastcall onSaveEDUStringS(const eduEntry* eduEntry);
	static void __fastcall initGlobalStrategyDirector(aiGlobalStrategyDirector* gsd);
	static int __fastcall onCreateUnit(char** entryName, const int* eduIndex);
	static void __fastcall onSetArmyGeneralsUnit(armyStruct* army);
	static int __fastcall onCalculateMiningIncome(int value, const settlementStruct* settlement);
	static int __fastcall onCreateMercUnitCheck(char** entryName, int eduIndex);
	static void __fastcall onWriteSoldiersToStrat(unit* unit);
	static int __fastcall onSetRegionHiddenResources(int hiddenResourceId, const mapRegion* region);
	static int __fastcall onReadHiddenResources(int hiddenResourceId, void* textBuffer);
	static int __fastcall onGetHiddenResource(const stringWithHash* name);
	static int __fastcall onCheckHiddenResource(int id, int region);
	static int __cdecl onGetWatchTowerRange();
	static void __fastcall threatDebugWalls(void *settAnalyzer);
	static int __fastcall onCallAuto(void *resolveData);
	static void __fastcall posSpecAttackGet(formationGame* form, formationData* data, uint32_t index, vector2* outPos);
	static uint32_t __fastcall onGetNormalPos(const formationData* data, uint32_t index);
	static int __fastcall onGeneralSiegeBug(const generalAssault* assault);
	static void __fastcall onCalculateUnitStats(unitStats* stats, uint32_t wpn, uint32_t armour);
	static void __fastcall onCalculateUnitStatsSoldier(unitStats* stats, const soldierInBattle* soldier);
	static void __fastcall onCalculateUnitStatsOfficer(generalInfo* officer);
	static void __fastcall onInitAutoUnit(autoResolveUnit* autoUnit);
	static void __fastcall onSetSettlementModel(settlementStruct* settlement);
	static int __fastcall onGeneralAssaultAction(generalAssault* assault);
	static int __fastcall onCalcBgSize(character* general, eduEntry* entry);
	static int __fastcall onFixPrec(unitTaskEngage* task);
	static void __fastcall onUpdatePhalanx(unitTaskMeleeAttackPhalanx* task);
	static void __fastcall onInitAttackMelee(soldierInBattle* soldier, actAttackMelee* task);
	static void __fastcall onUpdateBrace(soldierInBattle* soldier);
	static void __fastcall onSpearLocomotion(spearStruct* spear);
	static void __fastcall onAttackSpear(actAttackSpear* action);
	static void __fastcall onUpdateSpear(spearStruct* spear, bool placed, bool complete);
	static void __fastcall onProcessAttackMelee(actionInfo* info, actAttackMelee* task);
	static void __fastcall onInitControllers(aiPersonalityValues* personality);
	static void __fastcall onExitToMenu();
	static void __fastcall onRaiseSpear(spearStruct* spear, bool facingAway, int rank);
	static int __fastcall onCheckOwnership(uint32_t facShifted, const eduEntry* entry);
	static void __fastcall onSpawnArmy(factionStruct* faction, coordPair* coords, armyStruct* army);
	static unit* __fastcall onCreateGeneralUnit(unitDb* edu, character* general, int exp, int wpnlvl, int armlvl, eduEntry* entry); 
	static mountedEngine* __fastcall onGetMountedEngine(const stringWithHash* name);
	static bool __fastcall onParseMountedEngines(mountedEngineDb* db, descrParser* parser);
	static bool __fastcall onParseEdu(unitDb* db, descrParser* parser);
	static int __fastcall onElephantHitpointDamage();
	static int __fastcall onRenderUnitCardNumbers(unit* un);
	static int __fastcall onAddSettlementToDiplomacyScroll(const settlementStruct* settlement);
	static settlementStruct* __fastcall onDecideMissionTarget(settlementStruct* settlement, factionStruct* faction);
	static int __fastcall onCanWithdrawPreBattle(const settlementStruct* settlement);
	static int __cdecl onCalcArrowKillChance(unitStats* stats, soldierInBattle* defender, int16_t angle, int* dir, float heightDiff, int bonus);
	static int __fastcall onCalculateCommand(const characterRecord* general);
	static int __fastcall chooseCaptainUnit(armyStruct* army);
	static character* __fastcall onHealCasualties(battleSideArmy* army);
	static void __fastcall onRemoveFromUnitQueue(const unitRQ* queue, int index);
	static void __fastcall onLoadModelRigid(model_Rigid* model);
	static void __fastcall onUnloadModels();
	static building* __fastcall onGetBuildingById(const settlementBuildings* buildings, int index);
	static int __fastcall onCheckSettHasBuilding(const settlementBuildings* buildings, int index);
	static void __fastcall getPossibleConstructions(exportDescrBuildings* edb, settlementStruct* sett, void* data, void* caps, void* bonus, bool checkQueue, bool forceTemple);
	static int __fastcall onConflictTest(const buildingsQueue* queue, int index);
	static int __fastcall onOffMapModelThing(int culture);
	static DWORD* __fastcall onSetKhakiText(DWORD* text);
	static int __fastcall onAddBuildingCapsAfterConstruction(const settlementStruct* sett, int index);
	static building* __fastcall onCheckBuildUpgrade(const settlementStruct* sett, int buildingId);
	static void __fastcall onAttachRegionSettlement(settlementStruct* sett, int regionId);
	static int __fastcall onGetTrueBuildingCapabilities(int counter, const stackCapabilities* cap);
	static void __fastcall onCalculateSettlement(settlementStruct* sett);
	static eduEntry* __fastcall onCustomBattleCost(int eduIndexOffset);
	static int __fastcall onMarriageOption(const factionRecord* facRecord);
	static int __fastcall onCalcUnitStatsWpn(int weapon);
	static int __fastcall onCalcUnitValueArmour(int armour);
	static int* __fastcall onGetSupportingArmies(armyStruct* defender, armyStruct* attacker);
	static uint32_t __fastcall onDrawBanner(const trackedArmy* army);
	static bannerData* __fastcall onGetRebelSymbol(const trackedArmy* army, bannerData* oldData);
	static void __fastcall onPredictedStats(settlementStats* statsManager);
	static void* __fastcall onBattleCompleted();
	static int __fastcall onEvalAttObjective(const aiCampaignController* controller);
	static void __fastcall onCalculateLTGD(aiLongTermGoalDirector* ltgd);
	static void __fastcall onStartProductionTurn(aiPersonalityValues* personality);
	static factionStruct* __fastcall onCheckGarrison(const aiRegionController* controller);
	static int __fastcall onValidateGarrison(const aiRegionController* controller, const armyStruct* army);
	static void __fastcall onUpdateControllerAlloc(aiCampaignController* controller);
	static int __fastcall onScoreBestCapital(const settlementStruct* sett);
	static int __fastcall onGetUnitCard(const eduEntry* entry, int factionId, stringWithHash* newPath);
	static int __fastcall onGetUnitEduIndex(const stringWithHash* entryName);
	static int __fastcall onGetUnitInfoCard(const eduEntry* entry, int factionId, stringWithHash* newPath);
	static int __fastcall onAssessRequiredStrength(const aiRegionController* controller);
	static int __fastcall onCalcGarrisonStr(const aiRegionData* regData, const factionStruct* fac, const settlementStruct* sett);
	static DWORD* __fastcall onCreateTakeResidenceObjective(aiCampaignController* campaignController, DWORD* oldResidence);
	static void __fastcall onPreBattlePlacement(aiTacticAssault* aiTactic);
	static bool __thiscall onPreBattlePlacement2(aiUnitGroup* group, DWORD formationTemplate, bool forceOrder);
	static eduEntry* __fastcall onCreateMercUnit(char** entryName, eduEntry* entry);
	static eduEntry* __fastcall onCreateUnitWrapper(int eduIndexBase, int removeValue);
	static bool __fastcall onDecideRamAttacks(buildingBattle* gate, aiDetachment* detachment, int numRamsLeft);
	static const char* __fastcall onQuickSave();
	static const char* __fastcall onAutoSave();
	static void balanceMinorSettStats(settlementStats* stats, const settlementStruct* sett);
	static eduEntry* __fastcall onGetRecruitPoolUnitEntry(int eduIndex);
	static int __fastcall onFindUnit(char* entry, int* eduIndex);
	static uint32_t __fastcall onSetExtentsTexture(bool isEnemy);
	static uint8_t __fastcall onSetExtentsBorder(uint8_t isBorder, DWORD extentsDisplay);
	static uint32_t __fastcall onSetExtentsZoc();
	static uint8_t __fastcall onSetExtentsZocAlpha(uint8_t oldAlpha);
	static void __fastcall onDecideNeighbours(factionStruct* faction);
	static void __fastcall onTechtreeSlots(settlementBuildings* buildings, settlementStruct* sett, void* package);
	static void __fastcall onInitGsd(aiGlobalStrategyDirector* director);
	static void __fastcall onInitGsd2(aiGlobalStrategyDirector* director);
	static void __fastcall onSetBuildPolicies(aiProductionController* controller, int policy, int secondaryPolicy);
	static void __fastcall onUpdateProdControllers(aiPersonalityValues* personality);
	static void __fastcall onSetProdPriorities(aiProductionController* controller);
	static DWORD __fastcall onUnitInfo(DWORD entryAddress);
	static void __fastcall onTransferSettlement(const settlementStruct* settlement, int reason, factionStruct* faction);
	static portBuildingStruct* __fastcall onTransferSettlementPort(const settlementStruct* settlement);
	static int __fastcall onCheckConstructionItem(const buildingsQueue* queue, const buildingInQueue* constructionItem);
	static float __fastcall onCalculateUnitValue(eduEntry* entry, DWORD value);
	static int __fastcall onEvaluateUnit(int eduIndex);
	static DWORD __fastcall onCustomBattleUnitCards(DWORD cardArrayThing, int factionID);
	static int __fastcall onCustomBattleUnits(eduEntry** unitArray, int currentCount, int factionID);
	static int __fastcall onAttackGate(unit* unit, void* tactic);
	static eduEntry* __fastcall onEvaluateUnit2(int eduIndex);
	static eduEntry* __fastcall onReadDescrRebel(DWORD value);
	static DWORD __fastcall onSearchUnitType(char* typeName);
	static int __fastcall onReligionCombatBonus(int religionID, characterRecord* namedChar);
	static char* __fastcall getBrowserPicConstructed(int cultureID, edbEntry* entry, int buildingLevel);
	static char* __fastcall getBrowserPicConstruction(int cultureID, edbEntry* entry, int buildingLevel);
	static char* __fastcall getBuildingPic(buildingLevel* level, int cultureID);
	static char* __fastcall getBuildingPicConstructed(buildingLevel* level, int cultureID);
	static char* __fastcall getBuildingPicConstruction(buildingLevel* level, int cultureID);
	static portraitDbEntry* __fastcall getPortraitDbEntry(int cultureID);
	static char* __fastcall onGetGuildOfferPic(DWORD level, int cultureID);
	static unit** __fastcall onGetUnitByLabel(DWORD unitLabels, char* label);
	static unitGroup** __fastcall onGetGroupByLabel(DWORD groupLabels, char* label);
	static char* __fastcall onGetCultureEndTurnSound(int cultureID);
	static void __fastcall onCharacterClicked(character* enemy);
	static DWORD __fastcall onCampaignTick();
	static void __fastcall onBattleTick();


	static character* __fastcall mercenaryMovePointsGetGeneral(armyStruct* army);

	//click at tile
	static void __fastcall clickAtTile(coordPair* xy);

	//after loading of campaign map
	static void WINAPI afterCampaignMapLoaded();

	//after new game start
	static void WINAPI onNewGameStart();
	//after reading EDU
	static void WINAPI onEduParsed();
	static void WINAPI onGameInit();
	static void WINAPI onUnloadCampaign();
	static void WINAPI onNewGameLoaded();
	static void __fastcall onAiTurn(aiFaction* aiFac);
	
	//before start of a first faction turn
	static void WINAPI onChangeTurnNum();

	//on console command give_trait, after searching for character
	static void WINAPI onGiveTrait();

	//after loading campaign or battle data
	static void WINAPI afterLoadGameData();

	//after comparison in I_CompareCounter command
	static void WINAPI afterI_CompareCounter();

	//check fort for deleting
	static int WINAPI checkFort();

	//add suzerain dip state
	static int WINAPI checkDipString();
	static int WINAPI checkFacDip();

	//spawn_army command patch
	static int WINAPI spawnArmyCoords();
	static void WINAPI spawnEndArmyCoords();

	//spawn_character command patch
	static int WINAPI spawnCharacterCoords();

	//check and replace characters battle models before battle
	static void WINAPI onBattleStratScreen();


	//parse events in this functions
	static void __fastcall onEvent(DWORD** vTab, DWORD arg2);

	//load game
	static void __fastcall onLoadSaveFile(UNICODE_STRING**& savePath);
	//save game
	static void __fastcall onSaveGame(UNICODE_STRING**& savePath);

	//check of custom tile before battle, replace it here
	static void __fastcall onTileCheck(int* coords);
	//read custom tiles file and make right strings, structures, etc
	static void WINAPI readTilesFile();


	//read and load all startmap models
	static void WINAPI unicalStratModsRead();
	static void WINAPI loadCharStratModels();
	//moment before game start select stratmap models for drawing
	//replace models for the settlements, forts, ports, etc here without craches
	static void WINAPI checkAndChangeModels();
	
	static void WINAPI battleLoaded();
	static void WINAPI toReadGameDbsAtStart();
	
	static void WINAPI onDrawPartsOfStratObjects();
	static void WINAPI onDrawAll();
	static void WINAPI onStartOfDrawFunction();
	static void WINAPI onRetreat();


	static void __fastcall onStopCharacter(character* character);
	static void WINAPI onMoveRecruitQueue();
	static eduEntry* __fastcall recruitEopUnit(int eduIndex);
	static void __fastcall recruitEopUnit2(int eduIndex);
	static void __fastcall recruitEopMercUnit(DWORD pad, DWORD pad2, regionStruct* region, int eduIndex, int factionID, int exp);


	static void __fastcall onEndSiege(settlementStruct* sett);
	static void __fastcall onStartSiege(settlementStruct* sett);
	static void __fastcall onLoadDescrBattleCharacter(armyStruct* army, character* goalGen);


	//called not in all cases!
	static void WINAPI onBattleStateChange();

	static void __thiscall onFactionSymbolSelect();
	static void __thiscall onCharacterDied(character* character);
	static void __thiscall onFactionDied(factionStruct* faction);
};

class consolePatches
{
public:
	static void WINAPI onGameConsoleCommandFromConsole();
	static void WINAPI onGameConsoleCommandFromScript();

	static int __fastcall onReadLogonOrLogoff(int isLogonNow);
};


struct color8888
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
	uint32_t asInt() const
	{
		return (a << 24) | (r << 16) | (g << 8) | b;
	}
};

//### Can try to collect all the structures of the UI into a single structure.   
struct frameStruct
{
	int field_2[2];
	int posX;
	int posY;
	int	width;
	int	height;
};
struct advancedElementsStruct
{
	int field_2[2];
	frameStruct* frame;
};
struct advancedSettlementInfoScroll
{
	int field_4[4];
	int	width;
	int	height;
	int field_13[13];
	advancedElementsStruct* elements;
};

struct tableStruct
{
	int field_18[18];
	frameStruct* frame;
};
struct sliderStruct
{
	int field_5[5];
	int length; //0x0014 
	int field_8[8];
	int cursorPosX; //0x0038
	int cursorPosY;
	int xPos;
	int yPos;
	tableStruct* table; //0x0048
	int field_118[118];
	int minState;
	int maxState;
	int currentState; //0x022C 
	int field_16[16];
	int cursorPosX_2; //0x0270
	int cursorPosY_2; //0x0274 
	int xPosThumb; //0x0278 
	int yPosThumb; //0x027C 
	int field_99[99];
};
struct mercenaryIcon
{
	int field_14[14];
	int cursorPosX; //0x0038
	int cursorPosY;
	int posX;
	int posY;
};
struct mercenaryScroll
{
	int field_4[4];
	int	width; //0x0010
	int	height;
	int field_8[8];
	int cursorPosX; //0x0038
	int cursorPosY;
	int posX;
	int posY;
	tableStruct* table; //0x0048
	mercenaryIcon** mercs;
	int field_0050;
	int mercsNum;
	int field_43[43];
	sliderStruct* slider; //0x0104
};
struct imageStruct
{
	BYTE fields[0x00B4];
	const char* texturePath;
	int field_2[2];
	int	width; //0x00C0
	int	height;
};
struct detailsStruct
{
	tableStruct* table1; //0x0000
	tableStruct* table2; //0x0004
	imageStruct* img;  //0x0008
	void* plagueIcon;
};
struct direct3DTextureStruct
{
	void* pointer;
	int field_6[6];
	char* something;
	char field_3[3];
	int field_9[9];
	m_IDirect3DTexture9* texture1; //0x0048
	int field;
	m_IDirect3DTexture9* duplicate1; //0x0050
/*	///...
	m_IDirect3DTexture9* texture2; //0x04C8
	int field2;
	m_IDirect3DTexture9* duplicate2;
	///...
	m_IDirect3DTexture9* texture3; //0x0AC8
	int field3;
	m_IDirect3DTexture9* duplicate3;
	///...
	m_IDirect3DTexture9* texture4; //0x0BC8
	m_IDirect3DTexture9* texture5; //0x0BCC*/
public:
	m_IDirect3DTexture9* getGameTexture(int index) const
	{
		if (index == 1)
			return texture1;
	/*	else if (index == 2)
			return reinterpret_cast<m_IDirect3DTexture9*>(static_cast<char*>(pointer) + 0x04C8);
		else if (index == 3)
			return reinterpret_cast<m_IDirect3DTexture9*>(static_cast<char*>(pointer) + 0x0AC8);
		else if (index == 4)
			return reinterpret_cast<m_IDirect3DTexture9*>(static_cast<char*>(pointer) + 0x0BC8);
		else if (index == 5)
			return reinterpret_cast<m_IDirect3DTexture9*>(static_cast<char*>(pointer) + 0x0BCC);
		return NULL;*/
	}
};
struct characterInfoFrame
{
	int field_4[4];
	int	width; //0x0010
	int	height;
	int field_8[8];
	int cursorPosX; //0x0038
	int cursorPosY;
	int posX;
	int posY;
	tableStruct* table; //0x0048
	detailsStruct* details; //0x004C
	BYTE fields[0x02B0];
	int portraitFrameX; // 0x0300
	int portraitFrameY; // 0x0304
	int field; // 0x0308
	direct3DTextureStruct* textures; // 0x030C
};

struct rgba
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t alpha;
	explicit rgba(uint8_t b, uint8_t g, uint8_t r, uint8_t a) : red(r), green(g), blue(b), alpha(a) {}
	rgba() : blue(0), green(0), red(0), alpha(0)              {}
	void set(uint8_t r, uint8_t g, uint8_t b)                 { red = r; green = g; blue = b; }
	void set(uint8_t r, uint8_t g, uint8_t b, uint8_t a)      { red = r; green = g; blue = b; alpha = a; }
	void set(rgba* c)                                         { red = c->red; green = c->green; blue = c->blue; alpha = c->alpha; }
	bool cmp(uint8_t r, uint8_t g, uint8_t b) const           { return red == r && green == g && blue == b; }
	bool cmp(uint8_t r, uint8_t g, uint8_t b, uint8_t a) const{ return red == r && green == g && blue == b && alpha == a; }
	uint32_t getHex() const                                   { return (static_cast<uint32_t>(alpha) << 24) | (static_cast<uint32_t>(red) << 16) | (static_cast<uint32_t>(green) << 8) | blue; }
};

enum cameraState
{
	rotateLeft,
	rotateRight,
	rotateUp,
	rotateDown,
	stepLeft,
	stepRight,
	stepFwd,
	stepBack,
	unknown_1,
	unknown_2,
	unknown_3,
	unknown_4,
	unknown_5,
	unknown_6,
	unknown_7,
	unknown_8
};

//Perhaps in the future, if and when the number of functions increases, I will implement everything related to the camera in a separate file.
struct eopCamera
{
	bool isInit        = false;
	bool isRotated     = false;
	bool isMove        = false;
	bool isBattle      = false;
	bool isScrollOpen  = false;
	bool isReset       = false;
	bool toBattle      = false;
	bool camLog        = false;
	vector3 camPos     = {0.0f, 0.0f, 0.0f};
	vector3 camTarget  = {0.0f, 0.0f, 0.0f};
	float speedRotate  = 0.25f;
	float speedMove    = 0.25f;
	float speedZoom    = 0.25f;
	float offsetX1     = 0.0f;
	float offsetY1     = 0.0f;
	float offsetX2     = 0.0f;
	float offsetY2     = 0.0f;
	float offsetZ      = 0.0f;
	float offAerMapX   = 0.0f;
	float offAerMapY   = 0.0f;
	float minZ1        = 4.839232f;
	float maxZ1        = 33.348934f;
	float minZ2        = -10.363630f;
	float maxZ2        = 0.0f;
	float targetZ      = -1.0f;
	float targetX      = -1.0f;
	float targetY      = -1.0f;
	int screenX        = 1700;
	int screenY        = 1550;

	vector3* getPosition()     { return reinterpret_cast<vector3*>(codes::offsets.cameraPosition); }
	vector3* getTarget()       { return reinterpret_cast<vector3*>(codes::offsets.cameraTarget);   }
	void log(std::string text) { gameHelpers::logStringGame(text); };

	void move(float xOffset, float yOffset)
	{
		vector3* cameraPosition = getPosition();
		vector3* cameraTarget   = getTarget();

		float diffX = cameraTarget->x - cameraPosition->x;
		float diffY = cameraTarget->y - cameraPosition->y;
		float diffZ = cameraTarget->z - cameraPosition->z;

		float len = sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);

		if (len > 1e-6f) 
		{
			diffX /= len;
			diffY /= len;
			diffZ /= len;
		}
		else 
		{
			diffX = 0.0f;
			diffY = 1.0f;
			diffZ = 0.0f;
		}

		float rightX = diffY;
		float rightY = -diffX;
		float rightZ = 0.0f;

		float moveX = (diffX * yOffset + rightX * xOffset) * speedMove;
		float moveY = (diffY * yOffset + rightY * xOffset) * speedMove;
		float moveZ = (diffZ * yOffset + rightZ * xOffset) * speedMove;

		offsetX1  = moveX;
		offsetY1  = moveY;
		isMove    = true; 
		isRotated = false;
	}

	void moveX(float x) { move(x, 0.0f); }
	void moveY(float y) { move(0.0f, y); }
	void zoom(float z)  { offsetZ = speedZoom * z; isMove = true; isRotated = false; }

	void resetPos()
	{
		vector3* cameraPosition = getPosition();
		vector3* cameraTarget   = getTarget();
		cameraPosition->x = camPos.x;
		cameraPosition->y = camPos.y;
		cameraPosition->z = camPos.z;
		cameraTarget->x   = camTarget.x;
		cameraTarget->y   = camTarget.y;
		cameraTarget->z   = camTarget.z;
		isMove = true;

		if (camLog)
			log(
				"resetPos: \n | camPos: " + to_string(camPos.x) +          ", " + to_string(camPos.y) +          ", " + to_string(camPos.z) +
				"\n | camTarget: "        + to_string(camTarget.x) +       ", " + to_string(camTarget.y) +       ", " + to_string(camTarget.z) +
				"\n | cameraPosition: "   + to_string(cameraPosition->x) + ", " + to_string(cameraPosition->y) + ", " + to_string(cameraPosition->z) +
				"\n | cameraTarget: "     + to_string(cameraTarget->x) +   ", " + to_string(cameraTarget->y) +   ", " + to_string(cameraTarget->z)
			);
	}

	void reset(bool isKeypad8)
	{ 
		offsetX1  = 0.0f;
		offsetY1  = 0.0f;
		offsetX2  = 0.0f;
		offsetY2  = 0.0f;
		offsetZ   = 0.0f;
		targetZ   = -1.0f;
		targetX   = -1.0f;
		targetY   = -1.0f;
		isRotated = false;
		isMove    = isKeypad8;
		isReset   = isKeypad8;
	}

	void zoomCommand(vector3* camPos, vector3* camTarget)
	{
		if (targetZ < 0.0f)
			return;

		float gameMin = 0.2f; 
		float gameMax = 1.0f;

		float t = (targetZ - gameMin) / (gameMax - gameMin);
		t = max(0.0f, min(1.0f, t));

		float desiredZ = minZ1 + t * (maxZ1 - minZ1);
		float diff = desiredZ - camPos->z;
		if (abs(diff) < 0.5f) 
		{
			offsetZ = 0.0f;
			targetZ = -1.0f;
			return;
		}

		float moveZ = diff * 0.12f;
		if (abs(moveZ) > abs(diff))
			moveZ = diff;
		offsetZ = moveZ;

		if (camLog)
			log("zoomCommand:"
				"\n | targetZ: "   + to_string(targetZ) +
				"\n | t: "         + to_string(t) +
				"\n | desiredZ: "  + to_string(desiredZ) +
				"\n | camPos->z: " + to_string(camPos->z) +
				"\n | diff: "      + to_string(diff) +
				"\n | moveZ: "     + to_string(moveZ)
			);
	}

	void moveCommand(vector3* camPos, vector3* camTarget)
	{
		if (targetX < 0.0f || targetY < 0.0f)
			return;

		float diffX = camTarget->x - camPos->x;
		float diffY = camTarget->y - camPos->y;
		float diffZ = camTarget->z - camPos->z;
		float groundX, groundY;
		if (abs(diffZ) > 1e-6f)
		{
			float t = -camPos->z / diffZ;
			groundX = camPos->x + t * diffX;
			groundY = camPos->y + t * diffY;
		}
		else 
		{
			groundX = camTarget->x;
			groundY = camTarget->y;
		}

		float toX  = targetX - groundX;
		float toY  = targetY - groundY;
		float dist = sqrt(toX * toX + toY * toY);

		if (dist < 2.0f) 
		{
			targetX = -1.0f;
			targetY = -1.0f;
			return;
		}

		float gTime = GAME_FUNC(float(__cdecl*)(), updateTime)() / 2.0f;
		float moveX = toX * gTime;
		float moveY = toY * gTime;
		offsetX1    = moveX;
		offsetY1    = moveY;
	}

	void set(vector3& pos, vector3& tar)
	{
		if (!isMove)
			return;

		//If it is a full reset, we skip one frame, thereby resetting all camera parameters to their default values.   
		if (isReset)
		{
			isReset = false;
			return;
		}

		vector3* cameraPosition = getPosition();
		vector3* cameraTarget   = getTarget();

		moveCommand(cameraPosition, cameraTarget);
		zoomCommand(cameraPosition, cameraTarget);

		cameraPosition->x += offsetX1;
		cameraPosition->y += offsetY1;
		cameraTarget->x   += offsetX1;
		cameraTarget->y   += offsetY1;

		//If it's a transition to a battle.
		if (isBattle)
		{
			if (cameraPosition->z - 3.0f > minZ1)
			{
				cameraPosition->z -= 0.175f;
				cameraTarget->z   -= 0.175f;
			}
		}
		else 
		{
			if (cameraPosition->z + offsetZ < minZ1 /*|| cameraTarget->z + offsetZ < minZ2*/)
			{
				cameraPosition->z = minZ1;
			//	cameraTarget->z   = minZ2;
				targetZ = -1.0f;
			}
			else if (cameraPosition->z + offsetZ > maxZ1 /*|| cameraTarget->z + offsetZ > maxZ2*/)
			{
				cameraPosition->z = maxZ1;
			//	cameraTarget->z   = maxZ2;
				targetZ = -1.0f;
			}
			else 
			{
				cameraPosition->z += offsetZ;
				cameraTarget->z   += offsetZ / 2.75f;
			}
		}

		offsetX1 = 0.0f;
		offsetY1 = 0.0f;
		offsetZ  = 0.0f;

		battleDataS* battle = battleHelpers::getBattleData();
		if (!isBattle && battle && !battle->inBattle)
		{
			camPos    = *cameraPosition;
			camTarget = *cameraTarget;
		}

		if (camLog)
			log("set:"
				"\n | pos:            " + to_string(pos.x)                               + ", " + to_string(pos.y)                               + ", " + to_string(pos.z)                            +
				"\n | tar:            " + to_string(tar.x)                               + ", " + to_string(tar.y)                               + ", " + to_string(tar.z)                            +
				"\n | camPos:         " + to_string(camPos.x)                            + ", " + to_string(camPos.y)                            + ", " + to_string(camPos.z)                         +
				"\n | camTarget:      " + to_string(camTarget.x)                         + ", " + to_string(camTarget.y)                         + ", " + to_string(camTarget.z)                      +
				"\n | cameraPosition: " + to_string(cameraPosition->x)                   + ", " + to_string(cameraPosition->y)                   + ", " + to_string(cameraPosition->z)                +
				"\n | cameraTarget:   " + to_string(cameraTarget->x)                     + ", " + to_string(cameraTarget->y)                     + ", " + to_string(cameraTarget->z)                  +
				"\n | difference:     " + to_string(cameraTarget->x - cameraPosition->x) + ", " + to_string(cameraTarget->y - cameraPosition->y) + ", " + to_string(cameraTarget->z - cameraPosition->z)
			);

		pos = *cameraPosition;
		tar = *cameraTarget;
	}

	void rotate(void* gameCamera, float offset)
	{
		if (!gameCamera)
			return;

		isRotated = true;
		isMove    = false;
		offset *= speedRotate;

		float gTime = GAME_FUNC(float(__cdecl*)(), updateTime)();
		GAME_FUNC(void(__thiscall*)(void*, float, float, float), rotateStratCamera)(gameCamera, 0.0f, gTime * offset, 0.0f);

		vector3* cameraPosition = getPosition();
		vector3* cameraTarget   = getTarget();
		offsetX2   = cameraTarget->x - cameraPosition->x;
		offsetY2   = cameraTarget->y - cameraPosition->y;
		offAerMapX = offsetX2;
		offAerMapY = offsetY2;
		camPos     = *cameraPosition;
		camTarget  = *cameraTarget;

		if (camLog)
			log("rotate: position:    " + to_string(cameraPosition->x) + ", " + to_string(cameraPosition->y) + ", " + to_string(cameraPosition->z) +
				" | cameraTarget: "     + to_string(cameraTarget->x)   + ", " + to_string(cameraTarget->y)   + ", " + to_string(cameraTarget->z) +
				"\n | offsetX2: "       + to_string(offsetX2)          + " | offsetY2: " + to_string(offsetY2) 
			);
	}

	//This probably needs a bit of reworking.
	coordPair getAerialMapXY()
	{
	//	vector3* cameraPosition = getPosition();
		vector3* cameraTarget   = getTarget();

		float offX = 0.0f;
		float offY = 0.0f;

		if (!isBattle)
		{
			offX = offAerMapX / 2.0f;
			if (offAerMapY < 4.75f /*&& cameraPosition->z > minZ1 * 3.0f*/)
				offY = -(abs(offAerMapY) + 7.0f);
		}

		int x = static_cast<int>(ceil(cameraTarget->x + offX));
		int y = static_cast<int>(ceil(cameraTarget->y + offY));

		if (camLog)
			log("getAerialMapXY: offX: " + to_string(offX) + ", offY: " + to_string(offY) + ", x: " + to_string(x) + ", y: " + to_string(y));

		return coordPair(x, y);
	}

	void transitionToBattle()
	{
		isInit    = !toBattle;
		isBattle  = true;
		isMove    = true;
		isRotated = false;
		if (!isInit)
			return;
		getPosition()->z = maxZ1;
		getTarget()->z   = -maxZ1;
	}

	bool setPosition(int x, int y)
	{
		if (!isInit)
			return false;

		vector3* cameraPosition = getPosition();
		vector3* cameraTarget   = getTarget();

		float diffX = cameraTarget->x - cameraPosition->x;
		float diffZ = cameraTarget->z - cameraPosition->z;
		float diffY = cameraTarget->y - cameraPosition->y;

		float groundX, groundY;
		if (abs(diffZ) > 1e-6f)
		{
			float t = -cameraPosition->z / diffZ;
			groundX = cameraPosition->x + t * diffX;
			groundY = cameraPosition->y + t * diffY;
		}
		else 
		{
			groundX = cameraTarget->x;
			groundY = cameraTarget->y;
		}

		if (isBattle)
		{
			float horizLen = sqrt(diffX * diffX + diffY * diffY);
			if (horizLen > 1e-6f) 
			{
				float backX = -diffX / horizLen;
				float backY = -diffY / horizLen;
				float backOffset = 8.0f;
				groundX += backX * backOffset;
				groundY += backY * backOffset;
			}
		}

		offsetX1  = static_cast<float>(x) - groundX;
		offsetY1  = static_cast<float>(y) - groundY;
		isMove    = true;
		isRotated = false;

		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// MIN HOOK FUNCTIONS //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class minHookFunctions
{
public:
	static void init();
	static MH_STATUS hook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal, std::string function);
	static void draw();
	static void log(std::string text)                    { gameHelpers::logStringGame(text); };
	static std::string pointerToString(LPVOID ppPointer) { std::stringstream ss; ss << ppPointer; return ss.str(); }

	static bool openConsole;
	static oneTile* selectTile;
	static coordPair rememberCoords;

  
	using t_onUnitCreate = unit*(__thiscall*)(unitDb* _this, regionStruct* region, stringWithHash* id, int factionID, int combat_ability, int soldiers, int armour_lvl, int weapon_lvl);
	static t_onUnitCreate o_onUnitCreate;
	static unit* __thiscall onUnitCreate(unitDb* _this, regionStruct* region, stringWithHash* id, int factionID, int combat_ability, int soldiers, int armour_lvl, int weapon_lvl);
	static bool isUnlockWeaponLimit;
	
	using t_debugRenderLine = void(__cdecl*)(vector2* start, vector2* end, color8888 color, float time, bool zbuffered, float hOffset);
	static t_debugRenderLine o_debugRenderLine;
	static void __cdecl debugRenderLine(vector2* start, vector2* end, color8888 color, float time, bool zbuffered, float hOffset = 0.f);
	
	using t_debugRenderPeg = void(__cdecl*)(vector2* start, float height, color8888 color, float time);
	static t_debugRenderPeg o_debugRenderPeg;
	static void __cdecl debugRenderPeg(vector2* start, float height, color8888 color, float time);
	
	using t_displayMissileStats = int(__thiscall*)(eduEntry* entry, int exp, int wpn);
	static t_displayMissileStats o_displayMissileStats;
	static int __thiscall displayMissileStats(eduEntry* entry, int exp, int wpn);
	
	using t_displayMeleeStats = int(__thiscall*)(eduEntry* entry, int exp, int wpn);
	static t_displayMeleeStats o_displayMeleeStats;
	static int __thiscall displayMeleeStats(eduEntry* entry, int exp, int wpn);
	
	using t_displayArmourStats = int(__thiscall*)(eduEntry* entry, int armourUpg);
	static t_displayArmourStats o_displayArmourStats;
	static int __thiscall displayArmourStats(eduEntry* entry, int armourUpg);
	
	using t_displayDefenseStats = int(__thiscall*)(eduEntry* entry, int exp, int armourUpg);
	static t_displayDefenseStats o_displayDefenseStats;
	static int __thiscall displayDefenseStats(eduEntry* entry, int exp, int armourUpg);
	
	using t_fleeConstructor = void*(__thiscall*)(void* flee, void* sett, void* ch, bool action, bool back, bool clearMp);
	static t_fleeConstructor o_fleeConstructor1;
	static t_fleeConstructor o_fleeConstructor2;
	static void* __thiscall fleeConstructor1(void* flee, void* sett, void* ch, bool action, bool back, bool clearMp);
	static void* __thiscall fleeConstructor2(void* flee, void* sett, void* ch, bool action, bool back, bool clearMp);
	
	using t_debugRenderCircle = void(__cdecl*)(vector2* center, float radius, int segments, color8888 color, float time);
	static t_debugRenderCircle o_debugRenderCircle;
	static void __cdecl debugRenderCircle(vector2* center, float radius, int segments, color8888 color, float time);
	
	using t_debugLineAdd = void(__thiscall*)(vector2* start, vector2* end, color8888 color, float time, bool zbuffered, float hOffset);
	static t_debugLineAdd o_debugLineAdd;
	static int __thiscall debugLineAdd(void*, vector3* start, vector3* end, color8888 color, float time, bool zbuffered);

	using t_onMaybeWillSpyOpenGates = bool(__thiscall*)(void* _this, character* general);
	static t_onMaybeWillSpyOpenGates o_onMaybeWillSpyOpenGates;
	static bool __thiscall onMaybeWillSpyOpenGates(void* _this, character* general);

	using t_onCharacterSwitchFaction = void(__thiscall*)(character* _this, factionStruct* faction, int param_2, int param_3);
	static t_onCharacterSwitchFaction o_onCharacterSwitchFaction;
	static void __thiscall onCharacterSwitchFaction(character* _this, factionStruct* faction, int param_2, int param_3);

	using t_onCreateWife = characterRecord*(__thiscall*)(characterRecord* husband);
	static t_onCreateWife o_onCreateWife;
	static characterRecord* __thiscall onCreateWife(characterRecord* husband);

	using t_onCreateMessageAboutMarriage = void(__cdecl*)(characterRecord* husband, characterRecord* new_wife, marriageOption* mo);
	static t_onCreateMessageAboutMarriage o_onCreateMessageAboutMarriage;
	static void __cdecl onCreateMessageAboutMarriage(characterRecord* husband, characterRecord* new_wife, marriageOption* mo);
	static characterRecord* createWife(characterRecord* husband);

	using t_onCreateCandidateMarrying = characterRecord*(__thiscall*)(characterRecord* husband);
	static t_onCreateCandidateMarrying o_onCreateCandidateMarrying;
	static characterRecord* __thiscall onCreateCandidateMarrying(characterRecord* daughter);

	using t_onDaughterReadyMarryHusband = void(__cdecl*)(characterRecord* daughter, characterRecord* new_husband, marriageOption* mo);
	static t_onDaughterReadyMarryHusband o_onDaughterReadyMarryHusband;
	static void __cdecl onDaughterReadyMarryHusband(characterRecord* daughter, characterRecord* new_husband, marriageOption* mo);
	static characterRecord* createHusband(characterRecord* husband);

	using t_onCalculationRatioForBirth = float(__fastcall*)(family* _this);
	static t_onCalculationRatioForBirth o_onCalculationRatioForBirth;
	static float __fastcall onCalculationRatioForBirth(family* _this);

	using t_onBlockadePort = void(__thiscall*)(character* _this, portBuildingStruct* port);
	static t_onBlockadePort o_onBlockadePort;
	static void __thiscall onBlockadePort(character* _this, portBuildingStruct* port);
	static void createBlockadePortMessage(armyStruct* fleet, settlementStruct* settlement);

	using t_onQuickLoading = void(__cdecl*)(UNICODE_STRING**& savePath);
	static t_onQuickLoading o_onQuickLoading;
	static void __cdecl onQuickLoading(UNICODE_STRING**& savePath);

	using t_onUnitDestroy = void(__thiscall*)(armyStruct* _this, unit* unit);
	static t_onUnitDestroy o_onUnitDestroy;
	static void __thiscall onUnitDestroy(armyStruct* _this, unit* unit);

	using t_onCreateTooltip = void(__thiscall*)(DWORD _this, void* p, UNICODE_STRING*** u);
	static t_onCreateTooltip o_onCreateTooltip;
	static void __thiscall onCreateTooltip(DWORD _this, void* p, UNICODE_STRING*** u);
	static DWORD lastTooltipClass1;
	static void* lastTooltipClass2;
	static UNICODE_STRING*** lastTooltipUniString;
	static void resetTooltipText(const char* text);

	using t_onCreateTooltipByCoords = void(__thiscall*)(void* _this, coordPair* coords, UNICODE_STRING**& u1, UNICODE_STRING**& u2);
	static t_onCreateTooltipByCoords o_onCreateTooltipByCoords;
	static void __thiscall onCreateTooltipByCoords(void* _this, coordPair* coords, UNICODE_STRING**& u1, UNICODE_STRING**& u2);

	using t_onCreateCapabilityUnicodeString = UNICODE_STRING**&(__thiscall*)(buildingLevelCapability* _this, void* param_1, settlementStruct* settlement);
	static t_onCreateCapabilityUnicodeString o_onCreateCapabilityUnicodeString;
	static UNICODE_STRING**& __thiscall onCreateCapabilityUnicodeString(buildingLevelCapability* _this, void* param_1, settlementStruct* settlement);

	using t_onPlayGameSound = void(__cdecl*)(DWORD _this, int sound);
	static t_onPlayGameSound o_onPlayGameSound;
	static void __cdecl onPlayGameSound(DWORD _this, int sound);
	static DWORD lastSoundClass;

	using t_onPlayCharacterVoice = void(__cdecl*)(int sound, character* character, bool b);
	static t_onPlayCharacterVoice o_onPlayCharacterVoice;
	static void __cdecl onPlayCharacterVoice(int sound, character* character, bool b);

	using t_onHudElementClicked = void(__thiscall*)(int* param_1, int param_2, int param_3);
	static t_onHudElementClicked o_onHudElementClicked;
	static void __thiscall onHudElementClicked(int* param_1, int param_2, int param_3);

	using t_onSetElementPosition = void(__thiscall*)(int param_1, coordPair* coords, int param_3);
	static t_onSetElementPosition o_onSetElementPosition;
	static void __thiscall onSetElementPosition(int param_1, coordPair* coords, int param_3);
	static int arrowClick;
	static int arrowRememberY;
	static bool arrowChange;
	static int hudBuildings;

	using t_onCreateAdvancedSettlementInfoScroll = advancedSettlementInfoScroll*(__thiscall*)(advancedSettlementInfoScroll* _this, int width, int height);
	static t_onCreateAdvancedSettlementInfoScroll o_onCreateAdvancedSettlementInfoScroll;
	static advancedSettlementInfoScroll* __thiscall onCreateAdvancedSettlementInfoScroll(advancedSettlementInfoScroll* _this, int width, int height);
	static advancedSettlementInfoScroll* advancedSettlementInfoScrollClass;
	static void setAdvancedSettlementInfoScrollFrameSize(int width, int height);

	using t_onResetUnitRecruitAndRetrainScroll = void(__fastcall*)(DWORD _this);
	static t_onResetUnitRecruitAndRetrainScroll o_onResetUnitRecruitAndRetrainScroll;
	static void __fastcall onResetUnitRecruitAndRetrainScroll(DWORD _this);
	static DWORD unitRecruitAndRetrainScrollClass;
	static bool isBlockSlider;

	using t_onTraitsListConstructor1 = int*(__thiscall*)(int* _this, int width, int height);
	static t_onTraitsListConstructor1 o_onTraitsListConstructor1;
	static int* __thiscall onTraitsListConstructor1(int* _this, int width, int height);

	using t_onTraitsListConstructor2 = int*(__thiscall*)(int* _this, int width, int height);
	static t_onTraitsListConstructor2 o_onTraitsListConstructor2;
	static int* __thiscall onTraitsListConstructor2(int* _this, int width, int height);
	static int traitsChildWindowHeight;
	int static mercsMaxSlots;
	int static mercsNumOfLine;

	using t_onReinitNamedCharacterInfoScroll = void(__fastcall*)(int param_1);
	static t_onReinitNamedCharacterInfoScroll o_onReinitNamedCharacterInfoScroll;
	static void __fastcall onReinitNamedCharacterInfoScroll(int param_1);

	using t_onResetTraitsListHeight = void(__thiscall*)(int _this, int height);
	static t_onResetTraitsListHeight o_onResetTraitsListHeight;
	static void __thiscall onResetTraitsListHeight(int _this, int height);
	static bool isReset;

	using t_onMercenaryScrollInit = void(__thiscall*)(mercenaryScroll* _this, int param_2, int param_3, int param_4, int param_5, int param_6);
	static t_onMercenaryScrollInit o_onMercenaryScrollInit;
	static void __thiscall onMercenaryScrollInit(mercenaryScroll* _this, int param_2, int param_3, int param_4, int param_5, int param_6);

	using t_onSetSliderState = void(__thiscall*)(sliderStruct* _this, int sliderState, char param_3);
	static t_onSetSliderState o_onSetSliderState;
	static void __thiscall onSetSliderState(sliderStruct* _this, int sliderState, char param_3);
	static mercenaryScroll* lastMercScroll;
	static sliderStruct* lastSliderStruct;
	static bool isMercenaryScrollOpen;
	static map<int, int> bufferPosY;
	static void resetMercScrollCache()
	{
		lastMercScroll = nullptr;
		lastSliderStruct = nullptr;
		isMercenaryScrollOpen = false;
		bufferPosY.clear();
	}
	static bool isMercsNumValid() { return lastMercScroll && lastMercScroll->mercs && lastMercScroll->mercsNum > 0 && lastMercScroll->mercsNum <= 255; }

	using t_onElementReset = void(__fastcall*)(void* param_1);
	static t_onElementReset o_onElementReset;
	static void __fastcall onElementReset(void* param_1);
	static bool checkSetPosY(void* pointer);
	static bool checkActive() { return isMercenaryScrollOpen && lastMercScroll && lastMercScroll->slider && isMercsNumValid() &&
		lastMercScroll->cursorPosX > lastMercScroll->slider->xPos - 15 && lastMercScroll->cursorPosX < lastMercScroll->slider->xPos + 30 &&
		lastMercScroll->cursorPosY > lastMercScroll->slider->yPos - 30 && lastMercScroll->cursorPosY < lastMercScroll->slider->yPos + lastMercScroll->slider->length + 50;
	}

	using t_onSetColor = bool(__cdecl*)(rgba* color);
	static t_onSetColor o_onSetColor;
	static bool __cdecl onSetColor(rgba* color);
	static unordered_map<uint32_t, rgba> rgbaColors;
	static void createRgbaMap(rgba* targetColor, rgba* newColor) 
	{ 
		log("setRgbaTextColor: targetColor: " + to_string(targetColor->red) + ", " + to_string(targetColor->green) + ", " + to_string(targetColor->blue) + ", " + to_string(targetColor->alpha) + " --> newColor: " + to_string(newColor->red) + ", " + to_string(newColor->green) + ", " + to_string(newColor->blue) + ", " + to_string(newColor->alpha));
		rgbaColors[targetColor->getHex()] = *newColor;
	}
	static void checkSet(rgba* targetColor) {
		if (!targetColor || rgbaColors.size() == 0 || targetColor->cmp(255, 255, 255, 255) || targetColor->cmp(0, 0, 0, 255))
			return;
		if (auto it = rgbaColors.find(targetColor->getHex()); it != rgbaColors.end())
		{
			rgba* newColor = &it->second;
		//	log("checkSet: targetColor: " + to_string(targetColor->red) + ", " + to_string(targetColor->green) + ", " + to_string(targetColor->blue) + ", " + to_string(targetColor->alpha) + ", newColor: " + to_string(newColor->red) + ", " + to_string(newColor->green) + ", " + to_string(newColor->blue) + ", " + to_string(newColor->alpha));
			targetColor->set(newColor);
		}
	}
	static DWORD checkSet(DWORD targetHex) {
		if (auto it = rgbaColors.find(static_cast<uint32_t>(targetHex)); it != rgbaColors.end())
		{
			rgba* newColor = &it->second;
		//	log("checkSet: targetHex: #" + pointerToString((LPVOID*)targetHex) + ", newColor: " + to_string(newColor->red) + ", " + to_string(newColor->green) + ", " + to_string(newColor->blue) + ", " + to_string(newColor->alpha));
			return static_cast<DWORD>(newColor->getHex());
		}
		return targetHex;
	}

	using t_onCreateDebugInfoText = UNICODE_STRING**&(__thiscall*)(UNICODE_STRING**& _this, const char* str);
	static t_onCreateDebugInfoText o_onCreateDebugInfoText;
	static UNICODE_STRING**& __thiscall onCreateDebugInfoText(UNICODE_STRING**& _this, const char* str);
	static bool isDebugInfoOpen;
	static std::string debugInfoText;

	using t_onCharacterInfoFrameInit = void(__thiscall*)(characterInfoFrame* _this, characterRecord& nc, int param_3, int param_4, int param_5);
	static t_onCharacterInfoFrameInit o_onCharacterInfoFrameInit;
	static void __thiscall onCharacterInfoFrameInit(characterInfoFrame* _this, characterRecord& nc, int param_3, int param_4, int param_5);
	static direct3DTextureStruct* rememberTextures;

	using t_onCameraMove = void(__thiscall*)(void* _this, vector3* camPos, vector3* camTarget, bool some);
	static t_onCameraMove o_onCameraMove;
	static void __thiscall onCameraMove(void* _this, vector3* camPos, vector3* camTarget, bool some);

	using t_onCameraAction = void(__thiscall*)(void* _this, cameraState action, float speed);
	static t_onCameraAction o_onCameraAction;
	static void __thiscall onCameraAction(void* _this, cameraState action, float speed);
	
	using t_onAerialMapInit = int*(__thiscall*)(int* _this, vector3* maxHeightVec, float maxHeight);
	static t_onAerialMapInit o_onAerialMapInit;
	static int* __thiscall onAerialMapInit(int* _this, vector3* maxHeightVec, float maxHeight);

	using t_onAerialMapUpdate = bool(__thiscall*)(int _this, bool full);
	static t_onAerialMapUpdate o_onAerialMapUpdate;
	static bool __thiscall onAerialMapUpdate(int _this, bool full);

	using t_onAerialMapSetPosition = void(__thiscall*)(int _this, int x, int y);
	static t_onAerialMapSetPosition o_onAerialMapSetPosition;
	static void __thiscall onAerialMapSetPosition(int _this, int x, int y);

	using t_onMoveStratCameraSlow = void(__cdecl*)(int x, int y);
	static t_onMoveStratCameraSlow o_onMoveStratCameraSlow;
	static void __cdecl onMoveStratCameraSlow(int x, int y);

	using t_onZoomStratCamera = void(__cdecl*)(float target);
	static t_onZoomStratCamera o_onZoomStratCamera;
	static void __cdecl onZoomStratCamera(float target);

	using t_onCalcPathFindWidth = bool(__thiscall*)(void* _this, vector2* start, vector2* end, float maxWidth, float *outWidth, int swim, int maxSlope, int validZone);
	static t_onCalcPathFindWidth o_onCalcPathFindWidth;
	static bool __thiscall onCalcPathFindWidth(void* _this, vector2* start, vector2* end, float maxWidth, float *outWidth, int swim, int maxSlope, int validZone);

	static void stratCameraMouseWheel();
	static void enableEopCamera(bool set, float speedRotate, float speedMove, float speedZoom, bool toBattle);
	static void transitionToBattle(const int x, const int y);
	static bool isEopCameraEnable;
};

#define GET_VARIABLE_NAME(Variable) (#Variable)
#define MIN_HOOK(pTarget, pDetour, ppOriginal) minHookFunctions::hook((LPVOID)codes::offsets.pTarget, (LPVOID)pDetour, reinterpret_cast<void**>(&ppOriginal), GET_VARIABLE_NAME(pDetour))
#define MIN_HOOK_RAW(pTarget, pDetour, ppOriginal) minHookFunctions::hook((LPVOID)pTarget, (LPVOID)pDetour, reinterpret_cast<void**>(&ppOriginal), GET_VARIABLE_NAME(pDetour))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////







