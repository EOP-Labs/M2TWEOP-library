#include "pch.h"
#include "patchesForGame.h"
#include "tilesChange.h"
#include "graphicsD3D.h"


#include "events.h"
#include "fort.h"
#include "battle.h"
#include "cultures.h"
#include "eopBuildings.h"
#include "onlineThings.h"
#include "character.h"
#include "faction.h"
#include "characterRecord.h"
#include "strategyMap.h"
#include "campaignAi.h"

#include "plannedRetreatRoute.h"
#include "discordManager.h"
#include "eopdu.h"
#include "unit.h"
#include "army.h"
#include "campaign.h"
#include "gameUi.h"
#include "rebelFactions.h"
#include <memWork.h>

#include <algorithm>


worldRecord* __fastcall patchesForGame::selectWorldpkgdesc(char* database, worldRecord* selectedRecord)
{
	auto& battlemapWorker = globals::dataS.Modules.battleMapWorker;


	battlemapWorker.TryCreateRecodsList(reinterpret_cast<battlemapWorker::dataBaseWorlds*>(database));

	string selectRecordS = battlemapWorker.getRecordName(selectedRecord);
	string selectRecordG = battlemapWorker.getRecordGroup(selectedRecord);
	
	std::string retVal;
	std::string* tmpVal = gameEvents::onSelectWorldpkgdesc(selectRecordS.c_str(), selectRecordG.c_str());
	string selectedWorld = *tmpVal;
	delete tmpVal;
	if (selectedWorld.empty() || selectedWorld == selectRecordS)
	{
		selectedWorld = battleCreator::selectWorldpkgdesc(selectRecordS, selectRecordG);
		if (selectedWorld.empty() || selectedWorld == selectRecordS)
		{
			selectedWorld = globals::dataS.Modules.developerMode.selectWorldpkgdesc(selectRecordS, selectRecordG);
		}
	}

	if (selectedWorld.empty() || selectedWorld == selectRecordS)
	{
		return nullptr;
	}



	string selectedWorldCPP = selectedWorld;
	struct dataBaseS
	{
		int something;
		int something2;
		worldRecord* records;
		worldRecord* recordsEnd;
	};

	const dataBaseS* db = reinterpret_cast<dataBaseS*>(database);

	worldRecord* currRecord = db->records;
	do
	{
		string recName = battlemapWorker.getRecordName(currRecord);
		string recGroup = battlemapWorker.getRecordGroup(selectedRecord);
		if (recName == selectedWorldCPP && recGroup == selectRecordG)
		{
			return currRecord;
		}


		currRecord++;
	} while (currRecord != db->recordsEnd);

	return nullptr;
}
void __fastcall patchesForGame::onLoadSettlementWorldpkgdesc(worldRecord* selectedRecord)
{
	auto& battleMapWorker = globals::dataS.Modules.battleMapWorker;

	const string selectRecordS = battleMapWorker.getRecordName(selectedRecord);
	const string selectRecordG = battleMapWorker.getRecordGroup(selectedRecord);
	battleCreator::onLoadSettlementWorldpkgdesc(selectRecordS, selectRecordG);
}

float __fastcall patchesForGame::onCalculateUnitValue(eduEntry* entry, const DWORD value)
{
	float floatValue;
	std::memcpy(&floatValue, &value, sizeof(float));
	return gameEvents::onCalculateUnitValue(entry, floatValue);
}

struct nameIndexCombo
{
	nameIndexCombo(char* n, int i) : name(n), index(i) {}
	char* name{};
	int index{};
	nameIndexCombo() = default;
};

std::shared_ptr<nameIndexCombo> GLOBAL_NAME_INDEX_COMBO = std::make_shared<nameIndexCombo>();

DWORD __fastcall patchesForGame::onSearchUnitType(char* typeName)
{
	if (const auto eopUnit = eopDu::getEopEduEntryByName(typeName); eopUnit != nullptr)
	{
		//gameHelpers::logStringGame("Unit found in M2TWEOP: " + std::string(typeName) + " index: " + std::to_string(eopUnit->index));
		GLOBAL_NAME_INDEX_COMBO = std::make_shared<nameIndexCombo>(typeName, eopUnit->index);
		return reinterpret_cast<DWORD>(GLOBAL_NAME_INDEX_COMBO.get());
	}
	return 0;
}

int patchesForGame::onEvaluateUnit(const int eduIndex)
{
	return eopDu::getEduEntry(eduIndex)->unitProductionClass;
}

DWORD __fastcall patchesForGame::onCustomBattleUnitCards(const DWORD cardArrayThing, const int factionID)
{
	const int eopUnitNum = eopDu::getEopEntryNum();
	for (int i = 0; i < eopUnitNum; i++)
	{
		const auto eopUnit = eopDu::getEopEduEntryInternalIterating(i);
		if (!eopUnit->isFileAdded)
			continue;
		const auto entry = &eopUnit->data.edu;
		if (entry->category == unitCategory::nonCombatant || entry->category == unitCategory::ship)
			continue;
		if (GAME_FUNC(bool(__thiscall*)(eduEntry*, int, int), checkOwnershipCustom)(entry, factionID, 3))
		{
			stringWithHash card{};
			GAME_FUNC(void(__thiscall*)(eduEntry*, int, stringWithHash*), getUnitCard)(entry, factionID, &card);
			GAME_FUNC(void(__thiscall*)(DWORD, stringWithHash*), addToCardArray)(cardArrayThing, &card);
		}
	}
	return cardArrayThing;
}

void __fastcall patchesForGame::onSetBuildPolicies(aiProductionController* controller, int policy, int secondaryPolicy)
{
	controller->setBuildPoliciesAndTaxLevel(policy, secondaryPolicy);
}

void patchesForGame::onUpdateProdControllers(aiPersonalityValues* personality)
{
	personality->updateControllers();
}

void patchesForGame::onSetProdPriorities(aiProductionController* controller)
{
	controller->setPriorities();
}

void __fastcall patchesForGame::onDecideNeighbours(factionStruct* faction)
{
	faction->updateNeighbours();
}

portBuildingStruct* __fastcall patchesForGame::onTransferSettlementPort(const settlementStruct* settlement)
{
	if (settlement->isMinorSettlement)
		return nullptr;
	return settlement->port;
}

void __fastcall patchesForGame::onInitGsd(aiGlobalStrategyDirector* director)
{
	director->initialize();
}

void __fastcall patchesForGame::onInitGsd2(aiGlobalStrategyDirector* director)
{
	director->initialize2();
}

void __fastcall patchesForGame::onTransferSettlement(const settlementStruct* settlement, const int reason, factionStruct* faction)
{
	if (settlement->isMinorSettlement)
		return;
	const int watchTowerNum = stratMapHelpers::getRegion(settlement->regionID)->watchtowersNum;
	for (int i = 0; i < watchTowerNum; i++)
	{
		if (const auto watchTower = stratMapHelpers::getRegion(settlement->regionID)->watchtowers[i];
			!watchTower->blockingArmy)
		{
			GAME_FUNC(void(__thiscall*)(watchTowerStruct*, bool, factionStruct*, int)
				, changeWatchTowerFaction)(watchTower, reason != 1, faction, 0);
		}
	}
	GAME_FUNC(void(__thiscall*)(stratMap*, int, factionStruct*),
		setRegionFaction)(stratMapHelpers::getStratMap(), settlement->regionID, faction);
}

int __fastcall patchesForGame::onCheckConstructionItem(const buildingsQueue* queue, const buildingInQueue* constructionItem)
{
	const auto entry = constructionItem->edbEntry;
	if (queue->settlement->isMinorSettlement)
	{
		if (entry->isFarm /*|| entry->isPort*/ || entry->isTemple || (entry->isHinterland && !entry->isCoreBuilding))
			return 1; 
	}
	return queue->conversion > 0 ? 1 : 0;
}

DWORD* __fastcall patchesForGame::onCreateTakeResidenceObjective(aiCampaignController* campaignController, DWORD* oldResidence)
{
	if (*reinterpret_cast<DWORD*>(campaignController) != dataOffsets::offsets.campaignControllerVtbl)
		return oldResidence;
	if (*oldResidence == dataOffsets::offsets.settlementVtbl)
		return reinterpret_cast<DWORD*>(campaignController->regionData->getSettlement());
	return oldResidence;
}

void __fastcall patchesForGame::initGlobalStrategyDirector(aiGlobalStrategyDirector* gsd)
{
	
}

int __fastcall patchesForGame::onCustomBattleUnits(eduEntry** unitArray, int currentCount, const int factionID)
{
	const int eopUnitNum = eopDu::getEopEntryNum();
	for (int i = 0; i < eopUnitNum; i++)
	{
		if (currentCount >= 200)
			break;
		const auto eopUnit = eopDu::getEopEduEntryInternalIterating(i);
		if (!eopUnit->isFileAdded)
			continue;
		const auto entry = &eopUnit->data.edu;
		if (entry->category == unitCategory::nonCombatant
			|| entry->category == unitCategory::ship
			|| entry->noCustom != 0)
			continue;
		if (const int era = *reinterpret_cast<int*>(dataOffsets::offsets.selectedEra);
			GAME_FUNC(bool(__thiscall*)(eduEntry*, int, int), checkOwnershipCustom)(entry, factionID, era))
		{
			*unitArray = entry;
			unitArray++;
			currentCount++;
		}
	}
	return currentCount;
}

eduEntry* patchesForGame::onEvaluateUnit2(const int eduIndex)
{
	return eopDu::getEduEntry(eduIndex);
}

int __fastcall patchesForGame::onFortificationLevelS(settlementStruct* settlement, bool* isCastle)
{
	int selectedLevel = -2;//magic value, mean not change anything
	bool isChanged = false;
	int tmpVal = gameEvents::onFortificationLevelS(settlement, isCastle, &isChanged);
	if (isChanged == true)
	{
		selectedLevel = tmpVal;
	}
	if (selectedLevel == -2)
	{
		*isCastle = settlement->isCastle;
	}
	return selectedLevel;//use old thing
}

char* __fastcall patchesForGame::onSaveEDUStringS(const eduEntry* eduEntry)
{
	if (!eduEntry)
		return nullptr;
	const auto eopEntry = eopDu::getEopEduEntryInternal(static_cast<int>(eduEntry->index));
	if (eopEntry && !eopEntry->isFileAdded && !eopEntry->eopTypeName.empty())
	{
		return const_cast<char*>(eopEntry->eopTypeName.c_str());
	}
	return eduEntry->eduType;
}

int __fastcall patchesForGame::onCreateUnit(char** entryName, const int* eduIndex)
{
	if (eduIndex == nullptr)
	{
		const auto newEdu = eopDu::getEopEduEntryByName(*entryName);
		if (newEdu == nullptr)
			return -1;
		return newEdu->index;
	}
	return *eduIndex;
}

void patchesForGame::onSetArmyGeneralsUnit(armyStruct* army)
{
	if (army->gen && army->gen->bodyguards)
		army->generalsUnit = army->gen->bodyguards;
}

int patchesForGame::onCalculateMiningIncome(const int value, const settlementStruct* settlement)
{
	return campaignHelpers::modifyWithSettMechanics("SIF_MINING", static_cast<float>(value), settlement->isCastle == 1);
}

int __fastcall patchesForGame::onFindUnit(char* entry, int* eduIndex)
{
	if (eduIndex == nullptr)
	{
		if (entry == nullptr)
			return  -1;
		const auto newEdu = eopDu::getEopEduEntryByName(entry);
		if (newEdu == nullptr)
			return  -1;
		return newEdu->index;
	}
	return *eduIndex;
}

uint32_t __fastcall patchesForGame::onSetExtentsTexture(bool isEnemy)
{
	if (isEnemy)
		return extentColors::getEnemyColorHex();
	return extentColors::getOwnColorHex();
}

uint8_t __fastcall patchesForGame::onSetExtentsBorder(uint8_t isBorder, DWORD extentsDisplay)
{
	if (isBorder == 0)
		return 0;
	const DWORD extents = extentsDisplay - 0x24;
	const bool isEnemy = *reinterpret_cast<bool*>(extents + 0x80034);
	const bool border = isBorder == static_cast<uint8_t>(0xFF);
	if (isEnemy)
	{
		return border ? extentColors::getEnemyColor().borderAlpha : extentColors::getEnemyColor().alpha;
	}
	return border ? extentColors::getOwnColor().borderAlpha : extentColors::getOwnColor().alpha;
}

uint32_t __fastcall patchesForGame::onSetExtentsZoc()
{
	return extentColors::getZocColorHex();
}

uint8_t __fastcall patchesForGame::onSetExtentsZocAlpha(uint8_t oldAlpha)
{
	const float alphaDiff = oldAlpha / 200.0f;
	return static_cast<uint8_t>(extentColors::getZocColor().borderAlpha * alphaDiff);
}

int __fastcall patchesForGame::onReligionCombatBonus(int religionID, characterRecord* namedChar)
{
	if (religionID > 9)
		return 0;

	return namedChar->combatVsReligion[religionID];
}

std::string FILE_PATH;
std::string GAME_PATH;

std::string getBuildingPicPath(const char* cultureName, const char* levelName)
{
	FILE_PATH = "";
	FILE_PATH.append("/data/ui/");
	FILE_PATH.append(cultureName);
	FILE_PATH.append("/buildings/#");
	FILE_PATH.append(cultureName);
	FILE_PATH.append("_");
	FILE_PATH.append(levelName);
	FILE_PATH.append(".tga");
	return FILE_PATH;
}

std::string getBuildingPicConstructionPath(const char* cultureName, const char* levelName)
{
	FILE_PATH = "";
	FILE_PATH.append("/data/ui/");
	FILE_PATH.append(cultureName);
	FILE_PATH.append("/buildings/construction/#");
	FILE_PATH.append(cultureName);
	FILE_PATH.append("_");
	FILE_PATH.append(levelName);
	FILE_PATH.append(".tga");
	return FILE_PATH;
}

std::string getBuildingPicConstructedPath(const char* cultureName, const char* levelName)
{
	FILE_PATH = "";
	FILE_PATH.append("/data/ui/");
	FILE_PATH.append(cultureName);
	FILE_PATH.append("/buildings/#");
	FILE_PATH.append(cultureName);
	FILE_PATH.append("_");
	FILE_PATH.append(levelName);
	FILE_PATH.append("_constructed.tga");
	return FILE_PATH;
}

std::string getGamePath(const std::string& modPath) {
	size_t pos = modPath.length();
	int dirCount = 0;
	if (modPath.back() == '/' || modPath.back() == '\\') {
		pos--;
	}
	for (; pos > 0; pos--) {
		if (modPath[pos] == '/' || modPath[pos] == '\\') {
			dirCount++;
			if (dirCount == 2) {
				break;
			}
		}
	}
	return modPath.substr(0, pos);
}


char* __fastcall patchesForGame::getBrowserPicConstructed(int cultureID, edbEntry* entry, int buildingLevel)
{
	const auto level = entry->levels[buildingLevel];
	const auto levelName = level.name;
	if (levelName == nullptr)
		return nullptr;
	const auto cultureName = cultures::getCultureName(cultureID);
	if (cultureName.empty())
		return nullptr;
	const auto picPath = getBuildingPicConstructedPath(cultureName.c_str(), levelName);
	const auto modPath = gameHelpers::getModPath();
	FILE_PATH = modPath + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	if (GAME_PATH.empty())
		GAME_PATH = getGamePath(modPath);
	FILE_PATH = GAME_PATH + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	const auto edb = reinterpret_cast<exportDescrBuildings*>(dataOffsets::offsets.edbDataStart);
	const int variantNum = edb->lookupVariants.size();
	for (int i = 0; i < variantNum; i++)
	{
		const auto variant = edb->lookupVariants.get(i);
		const auto variantNamesSize = variant->names.size();
		for (int j = 0; j < variantNamesSize; j++)
		{
			const auto variantName = variant->names.get(j);
			const auto variantString = variantName->name;
			FILE_PATH = modPath + getBuildingPicConstructedPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
			FILE_PATH = GAME_PATH + getBuildingPicConstructedPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
		}
	}
	for (auto& i : level.buildingPicConstructed)
	{
		if (i.buildingPicPath != nullptr && i.picHash != 0)
		{
			FILE_PATH = i.buildingPicPath;
			return FILE_PATH.data();
		}
	}
	gameHelpers::logStringGame("getBrowserPicConstructed error: " + std::string(modPath + picPath));
	FILE_PATH = modPath + "/data/ui/generic/generic_constructed_building.tga";
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	FILE_PATH = GAME_PATH + "/data/ui/generic/generic_constructed_building.tga";
	return FILE_PATH.data();
}

char* __fastcall patchesForGame::getBrowserPicConstruction(int cultureID, edbEntry* entry, int buildingLevel)
{
	const auto level = entry->levels[buildingLevel];
	const auto levelName = level.name;
	if (levelName == nullptr)
		return nullptr;
	const auto cultureName = cultures::getCultureName(cultureID);
	if (cultureName.empty())
		return nullptr;
	const auto picPath = getBuildingPicConstructionPath(cultureName.c_str(), levelName);
	const auto modPath = gameHelpers::getModPath();
	FILE_PATH = modPath + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	if (GAME_PATH.empty())
		GAME_PATH = getGamePath(modPath);
	FILE_PATH = GAME_PATH + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	const auto edb = reinterpret_cast<exportDescrBuildings*>(dataOffsets::offsets.edbDataStart);
	const int variantNum = edb->lookupVariants.size();
	for (int i = 0; i < variantNum; i++)
	{
		const auto variant = edb->lookupVariants.get(i);
		const auto variantNamesSize = variant->names.size();
		for (int j = 0; j < variantNamesSize; j++)
		{
			const auto variantName = variant->names.get(j);
			const auto variantString = variantName->name;
			FILE_PATH = modPath + getBuildingPicConstructionPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
			FILE_PATH = GAME_PATH + getBuildingPicConstructionPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
		}
	}
	for (auto& i : level.buildingPicConstruction)
	{
		if (i.buildingPicPath != nullptr && i.picHash != 0)
		{
			FILE_PATH = i.buildingPicPath;
			return FILE_PATH.data();
		}
	}
	gameHelpers::logStringGame("getBrowserPicConstruction error: " + std::string(modPath + picPath));
	FILE_PATH = modPath + "/data/ui/generic/generic_preconstructed_building.tga";
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	FILE_PATH = GAME_PATH + "/data/ui/generic/generic_preconstructed_building.tga";
	return FILE_PATH.data();
}



char* __fastcall patchesForGame::getBuildingPic(buildingLevel* level, int cultureID)
{
	const auto levelName = level->name;
	if (levelName == nullptr)
		return nullptr;
	const auto cultureName = cultures::getCultureName(cultureID);
	if (cultureName.empty())
		return nullptr;
	const auto picPath = getBuildingPicPath(cultureName.c_str(), levelName);
	const auto modPath = gameHelpers::getModPath();
	FILE_PATH = modPath + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	if (GAME_PATH.empty())
		GAME_PATH = getGamePath(modPath);
	FILE_PATH = GAME_PATH + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	const auto edb = reinterpret_cast<exportDescrBuildings*>(dataOffsets::offsets.edbDataStart);
	const int variantNum = edb->lookupVariants.size();
	for (int i = 0; i < variantNum; i++)
	{
		const auto variant = edb->lookupVariants.get(i);
		const auto variantNamesSize = variant->names.size();
		for (int j = 0; j < variantNamesSize; j++)
		{
			const auto variantName = variant->names.get(j);
			const auto variantString = variantName->name;
			FILE_PATH = modPath + getBuildingPicPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
			FILE_PATH = GAME_PATH + getBuildingPicPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
		}
	}
	for (auto& i : level->buildingPic)
	{
		if (i.buildingPicPath != nullptr && i.picHash != 0)
		{
			FILE_PATH = i.buildingPicPath;
			return FILE_PATH.data();
		}
	}
	gameHelpers::logStringGame("getBuildingPic error: " + std::string(modPath + picPath));
	FILE_PATH = modPath + "/data/ui/generic/generic_building.tga";
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	FILE_PATH = GAME_PATH + "/data/ui/generic/generic_building.tga";
	return FILE_PATH.data();
}

char* __fastcall patchesForGame::getBuildingPicConstructed(buildingLevel* level, int cultureID)
{
	const auto levelName = level->name;
	if (levelName == nullptr)
		return nullptr;
	const auto cultureName = cultures::getCultureName(cultureID);
	if (cultureName.empty())
		return nullptr;
	const auto picPath = getBuildingPicConstructedPath(cultureName.c_str(), levelName);
	const auto modPath = gameHelpers::getModPath();
	FILE_PATH = modPath + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	if (GAME_PATH.empty())
		GAME_PATH = getGamePath(modPath);
	FILE_PATH = GAME_PATH + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	const auto edb = reinterpret_cast<exportDescrBuildings*>(dataOffsets::offsets.edbDataStart);
	const int variantNum = edb->lookupVariants.size();
	for (int i = 0; i < variantNum; i++)
	{
		const auto variant = edb->lookupVariants.get(i);
		const auto variantNamesSize = variant->names.size();
		for (int j = 0; j < variantNamesSize; j++)
		{
			const auto variantName = variant->names.get(j);
			const auto variantString = variantName->name;
			FILE_PATH = modPath + getBuildingPicConstructedPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
			FILE_PATH = GAME_PATH + getBuildingPicConstructedPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
		}
	}
	for (auto& i : level->buildingPicConstructed)
	{
		if (i.buildingPicPath != nullptr && i.picHash != 0)
		{
			FILE_PATH = i.buildingPicPath;
			return FILE_PATH.data();
		}
	}
	gameHelpers::logStringGame("getBuildingPicConstructed error: " + std::string(modPath + picPath));
	FILE_PATH = modPath + "/data/ui/generic/generic_constructed_building.tga";
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	FILE_PATH = GAME_PATH + "/data/ui/generic/generic_constructed_building.tga";
	return FILE_PATH.data();
}

char* __fastcall patchesForGame::getBuildingPicConstruction(buildingLevel* level, int cultureID)
{
	const auto levelName = level->name;
	if (levelName == nullptr)
		return nullptr;
	const auto cultureName = cultures::getCultureName(cultureID);
	if (cultureName.empty())
		return nullptr;
	const auto picPath = getBuildingPicConstructionPath(cultureName.c_str(), levelName);
	const auto modPath = gameHelpers::getModPath();
	FILE_PATH = modPath + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	if (GAME_PATH.empty())
		GAME_PATH = getGamePath(modPath);
	FILE_PATH = GAME_PATH + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	const auto edb = reinterpret_cast<exportDescrBuildings*>(dataOffsets::offsets.edbDataStart);
	const int variantNum = edb->lookupVariants.size();
	for (int i = 0; i < variantNum; i++)
	{
		const auto variant = edb->lookupVariants.get(i);
		const auto variantNamesSize = variant->names.size();
		for (int j = 0; j < variantNamesSize; j++)
		{
			const auto variantName = variant->names.get(j);
			const auto variantString = variantName->name;
			FILE_PATH = modPath + getBuildingPicConstructionPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
			FILE_PATH = GAME_PATH + getBuildingPicConstructionPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
		}
	}
	for (auto& i : level->buildingPicConstruction)
	{
		if (i.buildingPicPath != nullptr && i.picHash != 0)
		{
			FILE_PATH = i.buildingPicPath;
			return FILE_PATH.data();
		}
	}
	gameHelpers::logStringGame("getBuildingPicConstruction error: " + std::string(modPath + picPath));
	FILE_PATH = modPath + "/data/ui/generic/generic_preconstructed_building.tga";
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	FILE_PATH = GAME_PATH + "/data/ui/generic/generic_preconstructed_building.tga";
	return FILE_PATH.data();
}

portraitDbEntry* patchesForGame::getPortraitDbEntry(int cultureID)
{
	if (cultureID < 7)
	{
		const auto portraitDatabase = reinterpret_cast<portraitDb*>(dataOffsets::offsets.portraitDatabase);
		return &portraitDatabase->cultures[cultureID];
	}
	return cultures::eopPortraitDb::getEntry(cultureID);
}

char* patchesForGame::onGetGuildOfferPic(DWORD level, int cultureID)
{
	const DWORD lvlAddr = level - 8;
	const auto lvl = reinterpret_cast<buildingLevel*>(lvlAddr);
	const auto levelName = lvl->name;
	if (levelName == nullptr)
		return nullptr;
	const auto cultureName = cultures::getCultureName(cultureID);
	if (cultureName.empty())
		return nullptr;
	const auto picPath = getBuildingPicConstructedPath(cultureName.c_str(), levelName);
	const auto modPath = gameHelpers::getModPath();
	FILE_PATH = modPath + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	if (GAME_PATH.empty())
		GAME_PATH = getGamePath(modPath);
	FILE_PATH = GAME_PATH + picPath;
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	const auto edb = reinterpret_cast<exportDescrBuildings*>(dataOffsets::offsets.edbDataStart);
	const int variantNum = edb->lookupVariants.size();
	for (int i = 0; i < variantNum; i++)
	{
		const auto variant = edb->lookupVariants.get(i);
		const auto variantNamesSize = variant->names.size();
		for (int j = 0; j < variantNamesSize; j++)
		{
			const auto variantName = variant->names.get(j);
			const auto variantString = variantName->name;
			FILE_PATH = modPath + getBuildingPicConstructedPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
			FILE_PATH = GAME_PATH + getBuildingPicConstructedPath(variantString, levelName);
			if (std::filesystem::exists(FILE_PATH))
				return FILE_PATH.data();
		}
	}
	for (auto& i : lvl->buildingPicConstructed)
	{
		if (i.buildingPicPath != nullptr && i.picHash != 0)
		{
			FILE_PATH = i.buildingPicPath;
			return FILE_PATH.data();
		}
	}
	gameHelpers::logStringGame("onGetGuildOfferPic error: " + std::string(modPath + picPath));
	FILE_PATH = modPath + "/data/ui/generic/generic_preconstructed_building.tga";
	if (std::filesystem::exists(FILE_PATH))
		return FILE_PATH.data();
	FILE_PATH = GAME_PATH + "/data/ui/generic/generic_constructed_building.tga";
	return FILE_PATH.data();
}

unit** __fastcall patchesForGame::onGetUnitByLabel(DWORD unitLabels, char* label)
{
	if (!label || !label[0])
		return nullptr;

	if (const DWORD value = **reinterpret_cast<DWORD**>(label); value >= 0x01308DE4 && value <= 0x0135106C)
		return reinterpret_cast<unit**>(label);
	
	return GAME_FUNC(unit**(__thiscall*)(DWORD, char*), getUnitByLabel2)(unitLabels, label);
}

//12FFA84
unitGroup** __fastcall patchesForGame::onGetGroupByLabel(DWORD groupLabels, char* label)
{
	if (!label || !label[0])
		return nullptr;

	if (const DWORD value = **reinterpret_cast<DWORD**>(label); value == 0x12FFA84 || value == 0x01344AA4)
		return reinterpret_cast<unitGroup**>(label);
	
	return GAME_FUNC(unitGroup**(__thiscall*)(DWORD, char*), getGroupByLabel)(groupLabels, label);
}

namespace
{
	std::string CULTURE_SOUND;
}

char* patchesForGame::onGetCultureEndTurnSound(int cultureID)
{
	if (cultureID == 0)
		CULTURE_SOUND = "END_TURN";
	else
		CULTURE_SOUND = "END_TURN_CULTURE_" + to_string(cultureID);
	return CULTURE_SOUND.data();
}

int __fastcall patchesForGame::onCreateMercUnitCheck(char** entryName, int eduIndex)
{
	if (eduIndex == -1)
	{
		if (!eopDu::getEopEduEntryByName(*entryName))
			return -1;
		return 0;
	}
	return eduIndex;
}

void clearOfficers(unit* unit)
{
	if (unit->officer1)
	{
		GAME_FUNC(void(__thiscall*)(generalInfo*), officerDestructor)(unit->officer1);
		delete unit->officer1;
		unit->officer1 = nullptr;
	}
	if (unit->officer2)
	{
		GAME_FUNC(void(__thiscall*)(generalInfo*), officerDestructor)(unit->officer2);
		delete unit->officer2;
		unit->officer2 = nullptr;
	}
	if (unit->officer3)
	{
		GAME_FUNC(void(__thiscall*)(generalInfo*), officerDestructor)(unit->officer3);
		delete unit->officer3;
		unit->officer3 = nullptr;
	}
	if (unit->generalInfo)
	{
		GAME_FUNC(void(__thiscall*)(generalInfo*), officerDestructor)(unit->generalInfo);
		delete unit->generalInfo;
		unit->generalInfo = nullptr;
	}
}

void __fastcall patchesForGame::onWriteSoldiersToStrat(unit* unit)
{
	if (unit && unit->SoldierCountStrat == 0)
		unit->markedToKill = 1;
}

int patchesForGame::onSetRegionHiddenResources(const int hiddenResourceId, const mapRegion* region)
{
	eopHiddenResources::setInitialized(false);
	eopHiddenResources::addHiddenResourceToRegionIndex(region->name, hiddenResourceId);
	if (hiddenResourceId < 64)
		return 0;
	//gameHelpers::logStringGame("EOP Hidden resource with id: " + std::to_string(hiddenResourceId) + " added to region: " + region->name);
	return 1;
}

int patchesForGame::onReadHiddenResources(const int hiddenResourceId, void* textBuffer)
{
	//gameHelpers::logStringGame("Hidden resource with id: " + std::to_string(hiddenResourceId));
	if (hiddenResourceId > 63)
	{
		if (hiddenResourceId == 64)
		{
			const auto edb = eopBuildings::getEdb();
			for (int i = 0; i < edb->hiddenResourceCount; i++)
			{
				eopHiddenResources::addHiddenResourceWithId(edb->hiddenResources[i].hiddenResName, i);
			}
		}
		const char* name = GAME_FUNC(const char*(__thiscall*)(void*), getNextWord)(textBuffer);
		eopHiddenResources::addHiddenResource(name);
		return 1;
	}
	return 0;
}

int patchesForGame::onGetHiddenResource(const stringWithHash* name)
{
	return eopHiddenResources::getHiddenResourceIndex(name->name);
}

int patchesForGame::onCheckHiddenResource(const int id, const int region)
{
	return eopHiddenResources::hasHiddenResource(region, id);
}

int patchesForGame::onGetWatchTowerRange()
{
	return m2tweopOptions::getWatchTowerRange();
}

void patchesForGame::posSpecAttackGet(formationGame* form, formationData* data, uint32_t index, vector2* outPos)
{
	if (m2tweopOptions::getChangeGeneralPosition() && index == data->specEntries - 1)
	{
		const auto newIndex = data->entries + 1;
		callClassFunc<formationGame*, void, formationData*, uint32_t, vector2*>(form, 0x44, data, newIndex, outPos);
	}
	else
	{
		outPos->x = data->offsetColumnX * -1.0f - index * data->offsetRowX;
		outPos->y = data->offsetColumnY * -1.0f - index * data->offsetRowY;
	}
}

/*
void replaceGeneralUnitVtbl(unit* unit)
{
	gameHelpers::logStringGame("replaceGeneralUnitVtbl");
	if (!unit || !unit->generalArmy || !unit->generalArmy->soldier)
	{
		gameHelpers::logStringGame("replaceGeneralUnitVtbl - no unit or no general army or no soldier");
		return;
	}
	int formType = callClassFunc<formationGame*, int>(static_cast<formationGame*>(unit->formationsArray), 0x4);
	if (formType != 2)
	{
		gameHelpers::logStringGame("replaceGeneralUnitVtbl - invalid form type");
		return;
	}
	if (!FORMATION_CHANGED)
	{
		gameHelpers::logStringGame("creating new formation vtbl");
		FORMATION_CHANGED = new formationGame();
		auto existing = static_cast<formationGame*>(unit->formationsArray);
		DWORD newFunc = reinterpret_cast<DWORD>(&posSpecAttackGetVfunc);
		auto vtbl = existing->vtbl;
		auto oldPosFunc = vtbl->specPosGetFunc;
		//memcpy(FAKE_FORM_SQUARE_VTBL, reinterpret_cast<void*>(existing->vtbl), sizeof(fakeFormSquareVtbl));
		vtbl->specPosGetFunc = newFunc;
		//FORMATION_CHANGED->vtbl = reinterpret_cast<DWORD>(FAKE_FORM_SQUARE_VTBL);
	}
	gameHelpers::logStringGame("replaceGeneralUnitVtbl - vtbl replaced");
	//unit->formationsArray = FORMATION_CHANGED;
}*/

uint32_t patchesForGame::onGetNormalPos(const formationData* data, const uint32_t index)
{
	if (data->specEntries == 0 || !m2tweopOptions::getChangeGeneralPosition())
	{
		return index;
	}
	if (index == data->entries / 2)
	{
		return data->entries;
	}
	if (index == data->entries + 1)
	{
		return data->entries / 2;
	}
	return index;
}

int patchesForGame::onGeneralSiegeBug(const generalAssault* assault)
{
	if (!assault || !assault->character || !assault->character->army)
		return 1;
	return 0;
}

void patchesForGame::onCalculateUnitStats(unitStats* stats, const uint32_t wpn, const uint32_t armour)
{
	stats->setBattleStats(wpn, armour);
}

void patchesForGame::onCalculateUnitStatsSoldier(unitStats* stats, const soldierInBattle* soldier)
{
	stats->setBattleStats(soldier->soldierOnStratMap.weapon, soldier->soldierOnStratMap.armour);
}

void patchesForGame::onCalculateUnitStatsOfficer(generalInfo* officer)
{
	officer->stats.setBattleStats(officer->soldierOnStratMap.weapon, officer->soldierOnStratMap.armour);
}

void patchesForGame::onInitAutoUnit(autoResolveUnit* autoUnit)
{
	const auto eduEn = autoUnit->unit->eduEntry;
	if (eduEn->priStats.isMissile)
	{
		GAME_FUNC(void(__thiscall*)(statPri*, statPri*), priStatCopy)(&autoUnit->missileStats.priStats, &eduEn->priStats);
		autoUnit->ammoRemaining = eduEn->priStats.ammo;
		autoUnit->missileStats.chargeBonus = 0;
		autoUnit->missileStats.setBattleStats(autoUnit->unit->avgWeaponUpg, autoUnit->unit->avgArmourUpg);
		if (eduEn->secStats.isValid)
		{
			GAME_FUNC(void(__thiscall*)(statPri*, statPri*), priStatCopy)(&autoUnit->meleeStats.priStats, &eduEn->secStats);
			autoUnit->meleeStats.chargeBonus = eduEn->secStats.charge;
			autoUnit->meleeStats.setBattleStats(autoUnit->unit->avgWeaponUpg, autoUnit->unit->avgArmourUpg);
		}
	}
	else
	{
		GAME_FUNC(void(__thiscall*)(statPri*, statPri*), priStatCopy)(&autoUnit->meleeStats.priStats, &eduEn->priStats);
		autoUnit->meleeStats.chargeBonus = eduEn->priStats.charge;
		autoUnit->meleeStats.setBattleStats(autoUnit->unit->avgWeaponUpg, autoUnit->unit->avgArmourUpg);
		autoUnit->missileStats.priStats.isValid = false;
		autoUnit->ammoRemaining = 0;
	}
}

void patchesForGame::onSetSettlementModel(settlementStruct* settlement)
{
	bool changed = false;
	if (settlement->regionID > -1)
	{
		if (const auto settData = eopSettlementDataDb::get()->getSettlementData(settlement->regionID, settlement->minorSettlementIndex);
			settData && settData->modelId && settData->modelId > -1)
		{
			if (const auto modelRecord = stratModelsChange::findStratModel(settData->modelId); modelRecord && modelRecord->modelP)
			{
				stratModelsChange::changeSettlementStratModel(settlement, modelRecord->modelP);
				changed = true;
			}
		}
	}
	if (!changed)
	{
		const auto cultSett = cultures::getCultureSettlement(settlement->level, settlement->fac_creatorModNum);
		settlement->model = cultSett;
	}
}

int patchesForGame::onGeneralAssaultAction(generalAssault* assault)
{
	if (!assault || !assault->settlement)
		return 1;
	if (!assault->settlement->army)
	{
		gameHelpers::logStringGame("GeneralAssaultAction: No army in settlement: " + std::string(assault->settlement->name));
		assault->finished = true;
		assault->init = false;
		return 0;
	}
	return 1;
}

int patchesForGame::onCalcBgSize(character* general, eduEntry* entry)
{
	return gameHelpers::calculateMaxBodyguardSize(general, entry);
}

int patchesForGame::onFixPrec(unitTaskEngage* task)
{
	if (task->task.unit->unitPositionData->engagedUnitsNum > 0)
		task->task.mode = 0;
	else
		task->task.mode = 6;
	
	const auto bitfield = reinterpret_cast<uint32_t>(task) + 4;
	return *reinterpret_cast<uint32_t*>(bitfield);
}

void patchesForGame::onUpdatePhalanx(unitTaskMeleeAttackPhalanx* task)
{
	vector2 top{};
	top.x = 0.0f;
	top.y = 0.0f;

	const auto un = task->task.unit;
	const auto formation = un->formationsArray;
	const auto formationData = &un->offsetRowX;

	callClassFunc<void*, void, float*, vector2*>(formation, 0xF4, formationData, &top);
	float standoff = 0.5f;
	/*
	if (un->SoldierCountBattlemap)
	{
		const auto soldier = un->soldiersBattleArr[0];
		if (soldier->spear)
		{
			constexpr float spearMargin = 0.75f;
			vector3 spearTip{};
			spearTip.x = 0.0f;
			spearTip.y = 0.0f;
			spearTip.z = 0.0f;

			GAME_FUNC(int (__cdecl*)(vector3*, soldierInBattle*, int), getImpactPoint)(&spearTip, soldier, 608);
			float reach = sqrtf(spearTip.x * spearTip.x + spearTip.z * spearTip.z);
			standoff = max(standoff, reach - spearMargin);
		}
	}
	*/
	//top.x += 0.25f;
	top.y += standoff;
	const auto sineValues = battleHelpers::getSineValues();
	const auto sine = sineValues[task->angle >> 2];
	task->formationOriginY = task->targetPosY - ((top.y * sineValues[(task->angle + 0x4000) >> 2]) - (sine * top.x));
	task->formationOriginX = task->targetPosX - ((top.y * sine) + (sineValues[(task->angle + 0x4000) >> 2] * top.x));
}

void patchesForGame::onInitAttackMelee(soldierInBattle* soldier, actAttackMelee* task)
{
	if (soldier->spear)
		task->usePrimary = 1;
}

void patchesForGame::onUpdateBrace(soldierInBattle* soldier)
{
	soldier->updateBrace();
}

void patchesForGame::onSpearLocomotion(spearStruct* spear)
{
	if (const auto braceType = spear->soldier->braceType; braceType == 1)
		spear->moveState = spear->shuffleState;
}

void patchesForGame::onAttackSpear(actAttackSpear* action)
{
	const auto soldier = action->baseAction.soldier;
	const auto spear = soldier->spear;

	constexpr float arrivedDistSq = 10.f;
	const auto soldierPos = soldier->get2dPos();
	const vector2 tmp = vector2(soldierPos.x - action->posX, soldierPos.x - action->posY);
	const float dist = tmp.x * tmp.x + tmp.y * tmp.y;
	if (dist < arrivedDistSq
		&& spear
		&& GAME_FUNC(bool(__thiscall*)(soldierInBattle*), isSpearActive)(soldier)
		)
	{
		auto idleState = spear->idleState;
		if (soldier->locomotiveElement.currentStateGroupID == spear->moveState.groupId)
		{
			callClassFunc<soldierInBattle*, void, locomotionId*>(soldier, 0xb8, &idleState);
		}
		return;
	}

	locomotionId moveState;
	moveState.groupId = 4;
	moveState.stateId = 2;
	if (spear)
	{
		moveState = spear->moveState;
	}
	struct location
	{
		void* ref;
		float posX;
		float posY;
		float posZ;
		uint16_t angle;
		location(float x, float z, uint16_t angle) : posX(x), posY(0.0f), posZ(z), angle(angle), ref(nullptr) {}
	};
	auto loc = location(action->posX, action->posY, action->rotation);
	callClassFunc<soldierInBattle*, void, location*, locomotionId*, uint32_t>(soldier, 0xb4,
		&loc, &moveState, (1 << 2) | (1 << 8));
}

void patchesForGame::onUpdateSpear(spearStruct* spear, bool placed, bool complete)
{
	spear->update(placed, complete);
}

void patchesForGame::onProcessAttackMelee(actionInfo* info, actAttackMelee* task)
{
	const auto soldier = task->getSoldier();
	if (soldier->spear)
	{
		const auto underThreat = GAME_FUNC(bool(__cdecl*)(soldierInBattle*, actionInfo*), isThreatened)(task->getSoldier(), info);
		task->usePrimary = !soldier->usingSecondaryWeapon && !underThreat && info->enemyNum <= phalanxOptions::getInstance()->maxSoldiersSecondary;
	}
}

void patchesForGame::onInitControllers(aiPersonalityValues* personality)
{
	personality->initControllers();
}

void patchesForGame::onExitToMenu()
{
	gameHelpers::logStringGame("GAME EXIT TO MENU - CLEAR DATA");
	minorSettlementDb::clear();
	eopSettlementDataDb::get()->clearData();
	settlementHelpers::clearOptionsDb();
	plugData::data.luaAll.clearHashMapsCampaign();
	gameHelpers::logStringGame("DATA CLEARED");
	gameEvents::onExitToMenu();
	gameHelpers::logStringGame("EXIT TO MENU EVENTS PROCESSED");
}

void patchesForGame::onRaiseSpear(spearStruct* spear, bool facingAway, int rank)
{
	if (!spear->raised || !facingAway)
		return;
	const auto formationType = spear->unit->getUnitFormation();
	bool inPhalanx = formationType == formation::phalanx ||
		(spear->unit->eduEntry->priStats.isShortPike && spear->unit->actionStatus != unitStatus::routing);
	bool inReadyMode = spear->unit->unitPositionData->closestEnemyUnitDist < 60.f;
	bool advancingInFormation = (inPhalanx || formationType == formation::schiltrom) && inReadyMode && rank < 4;
	if (advancingInFormation)
		spear->raised = false;
}

int patchesForGame::onCheckOwnership(const uint32_t facShifted, const eduEntry* entry)
{
	if (m2tweopOptions::getIgnoreOwnershipRecruitment())
		return 1;
	return entry->ownership & facShifted;
}

void patchesForGame::onSpawnArmy(factionStruct* faction, coordPair* coords, armyStruct* army)
{
	GAME_FUNC(void (__thiscall*)(factionStruct*, coordPair*), factionResurrectStuffFunc)(faction, coords);
	gameEvents::onSpawnArmy(army);
}

unit* patchesForGame::onCreateGeneralUnit(unitDb* edu, character* general, int exp, int wpnlvl, int armlvl,
                                          eduEntry* entry)
{
	return eopDu::createGeneralUnit(general, exp, wpnlvl, armlvl, entry);
}

mountedEngine* patchesForGame::onGetMountedEngine(const stringWithHash* name)
{
	if (!name || !name->name)
		return nullptr;
	return eopMountedEngineDb::get()->getMountedEngine(std::string(name->name));
}

bool patchesForGame::onParseMountedEngines(mountedEngineDb* db, descrParser* parser)
{
	return db->parse(parser);
}

bool patchesForGame::onParseEdu(unitDb* db, descrParser* parser)
{
	return db->parse(parser);
}

int patchesForGame::onElephantHitpointDamage()
{
	return m2tweopOptions::getElephantHitpointDamage();
}

int patchesForGame::onRenderUnitCardNumbers(unit* un)
{
	if (m2tweopOptions::getDrawUnitCardOutline())
		return 1;
	return un->general ? 1 : 0;
}

int patchesForGame::onAddSettlementToDiplomacyScroll(const settlementStruct* settlement)
{
	if (settlement->isMinorSettlement)
		return 0;
	return 1;
}

settlementStruct* patchesForGame::onDecideMissionTarget(settlementStruct* settlement, factionStruct* faction)
{
	if (settlement->isMinorSettlement)
		return nullptr;
	if (!settlement->isEnemyToFaction(faction))
		return nullptr;
	return settlement;
}

int patchesForGame::onCanWithdrawPreBattle(const settlementStruct* settlement)
{
	if (settlement && settlement->siegeNum > 0)
		return 1;
	if (const auto battle = battleHelpers::getBattleData();
	 battle && battle->battleState > 0 && battle->battleType == battleType::siege)
	{
		return 1;
	}
	return 0;
}

namespace
{
	constexpr int piDiv2 = 0x4000;
	struct flank
	{
		int16_t start;
		int16_t end;
	};
	flank FLANK_LEFT {-piDiv2 + (piDiv2 / 4), -piDiv2 - (piDiv2 / 4)};
	flank FLANK_RIGHT { piDiv2 - (piDiv2 / 4), piDiv2 + (piDiv2 / 4)};
}

int patchesForGame::onCalcArrowKillChance(unitStats* stats, soldierInBattle* defender, int16_t angle, int* dir,
	float heightDiff, int bonus)
{
	int attack = static_cast<int>(heightDiff * 0.2);
	attack = clamp(attack, -4, 4);

	if (defender->isInFormation)
	{
		if (const auto formationType = callClassFunc<void*, int>(defender->thisUnit->formationsArray, 0x4);
			formationType == formation::phalanx)
		{
			attack -= m2tweopOptions::getPhalanxMissileDefense();
		}
		else if (formationType == formation::schiltrom)
		{
			attack -= m2tweopOptions::getSchiltromMissileDefense();
		}
		else if (formationType == formation::shieldWall)
		{
			attack -= m2tweopOptions::getShieldWallMissileDefense();
		}
	}
	attack += GAME_FUNC(int (__cdecl*)(unitStats*), getRainLevelArrowEffect)(stats);

	auto tile = defender->thisObject.battleTile;
	if (defender->mount)
		tile = defender->mount->thisObject.battleTile;
	
	if (tile)
	{
		if (const auto groundType = tile->physicalGroundType;
			groundType == battleGroundType::forestDense || groundType == battleGroundType::wood)
		{
			attack -= 6;
		}
	}

	auto armour = defender->stats.armourInBattle;
	attack += stats->priStats.attack;

	if (stats->priStats.isAP)
	{
		armour >>= 1;
		if (stats->priStats.weaponTecType == weaponTechType::missile_gunpowder)
			armour >>= 1;
	}

	attack -= armour;
	
	const auto attackAngle = angle - defender->thisObject.rotation + 0x7FFF;
	int attackDir = 0;
	if (attackAngle <= 0)
	{
		if ( attackAngle >= FLANK_LEFT.end )
			attackDir = attackAngle < FLANK_LEFT.start;
		else
			attackDir = 2;
	}
	else if ( attackAngle <= FLANK_RIGHT.end )
	{
		attackDir = attackAngle <= FLANK_RIGHT.start ? 0 : 3;
	}
	else
	{
		attackDir = 2;
	}

	auto shield = defender->stats.armourStats.shield;
	if (stats->priStats.isAP && m2tweopOptions::getApIsShieldPiercing())
		shield >>= 1;
	if (stats->priStats.weaponTecType == weaponTechType::missile_gunpowder || stats->priStats.isShieldPiercing())
		shield >>= 1;

	if (attackDir)
	{
		if ( attackDir == 1 || attackDir == 3 || attackDir == 5 )
			attack += shield / -2;
	}
	else
	{
		attack -= shield;
	}

	if (stats->priStats.isThrown)
	{
		const auto elephant = callClassFunc<soldierInBattle*, soldierInBattle*>(defender, 0x24);
		if (elephant)
		{
			if (const auto mountType = callClassFunc<soldierInBattle*, int>(elephant, 0x128); mountType == mountClass::elephant)
				attack += m2tweopOptions::getElephantJavBonus();
		}
	}
	
	attack += bonus;
	if (attack < -6)
	{
		if ( attack < -12 )
			attack = 4 * attack + 68;
		else
			attack = 2 * (5 * attack + 70);
	}
	else
	{
		attack = 4 * (5 * attack + 50);
	}
	attack = clamp(attack, 1, 990);

	if (stats->priStats.weaponType != weaponType::siegeMissile)
		attack = GAME_FUNC(int(__cdecl*)(int, soldierInBattle*), missileBalancingAdjustment)(attack, defender);

	return attack;
}

int patchesForGame::onCalculateCommand(const characterRecord* general)
{
	if (general && general->gen && general->gen->getTypeID() == characterTypeStrat::admiral)
		return general->navalCommand;
	return 0;
}

void patchesForGame::onRemoveFromUnitQueue(const unitRQ* queue, const int index)
{
	if (!queue || !queue->settlement)
		return;
	const auto item = queue->settlement->getUnitInQueue(index);
	gameEvents::onRemoveFromUnitQueue(item);
}

void patchesForGame::onLoadModelRigid(model_Rigid* model)
{
	stratModelsChange::pushGameModel(model);
}

void patchesForGame::onUnloadModels()
{
	stratModelsChange::clearModels();
}

building* patchesForGame::onGetBuildingById(const settlementBuildings* buildings, const int index)
{
	if (index < 0)
		return nullptr;
	for (int i = 0; i < buildings->buildingsNum; i++)
	{
		if (buildings->buildings[i]->edbEntry->buildingID == index)
			return buildings->buildings[i];
	}
	return nullptr;
}

int patchesForGame::onCheckSettHasBuilding(const settlementBuildings* buildings, int index)
{
	if (index < 0)
		return 0;
	for (int i = 0; i < buildings->buildingsNum; i++)
	{
		if (buildings->buildings[i]->edbEntry->buildingID == index)
			return 1;
	}
	return 0;
}

void patchesForGame::onTechtreeSlots(settlementBuildings* buildings, settlementStruct* sett, void* package)
{
	if (!buildings->buildingsNum)
	{
		return;
	}
	const auto facId = sett->fac_creatorModNum;
	edbEntry* entries[2048];
	const auto edb = eopBuildings::getEdb();
	const int total = GAME_FUNC(int(__thiscall*)(exportDescrBuildings*, int8_t, edbEntry**), getEntriesByFaction)(edb, static_cast<int8_t>(facId), entries);
	if (total > 0)
	{
		std::vector<edbEntry*> buildingEntries;
		buildingEntries.reserve(total);
		for (int b = 0; b < total; ++b)
		{
			const auto currentEntry = entries[b];
			if (GAME_FUNC(bool(__thiscall*)(edbEntry*, int8_t, int, int, bool), isLevelAvailableForLevel)(currentEntry, static_cast<int8_t>(facId), sett->level, 0, sett->isCastle))
			{
				buildingEntries.push_back(currentEntry);
			}
		}

		bool temple = false, guild = false;
		for (const auto& buildingEntry : buildingEntries)
		{
			if (buildingEntry->classification == 5)
			{
				if (guild)
					continue;
				guild = true;
			}
			if (buildingEntry->isTemple)
			{
				if (temple)
					continue;
				temple = true;
			}
			if (buildingEntry->isPort)
			{
				const auto region = stratMapHelpers::getRegion(sett->regionID);
				if (!GAME_FUNC(bool(__thiscall*)(regionStruct*), regionCanHavePort)(region))
					continue;
			}
			if (buildingEntry->buildingLevelCount > 0)
			{
				int highest = -1;
				for (int l = 0; l < buildingEntry->buildingLevelCount; ++l)
				{
					if (GAME_FUNC(bool(__thiscall*)(edbEntry*, int8_t, int, int, bool), isLevelAvailableForLevel)(buildingEntry, static_cast<int8_t>(facId), sett->level, l, sett->isCastle))
					{
						highest = max(l, highest);
					}
				}
				if (highest != -1)
				{
					if (const auto build = GAME_FUNC(building* (__thiscall*)(exportDescrBuildings*, settlementStruct*, edbEntry*, bool), createBuildInSett)(eopBuildings::getEdb(), sett, buildingEntry, false); build)
					{
						GAME_FUNC(void(__thiscall*)(building*, int, int, bool), upgradeBuildInSett)(build, highest, facId, false);
					}
				}
			}
		}
	}
}

void patchesForGame::getPossibleConstructions(exportDescrBuildings* edb, settlementStruct* sett, void* data, void* caps,
	void* bonus, const bool checkQueue, const bool forceTemple)
{
	int facId = sett->faction->factionID;
	if (sett->faction->factionRecord->slave && sett->subFactionID < 31)
		facId = sett->subFactionID;
	edbEntry* entries[2048];
	const int total = GAME_FUNC(int(__thiscall*)(exportDescrBuildings*, int8_t, edbEntry**), getEntriesByFaction)(edb, static_cast<int8_t>(facId), entries);
	const DWORD context = techFuncs::allocateGameMem(0x840);
	GAME_FUNC(void(__thiscall*)(DWORD, settlementStruct*, void*), createSettlementConditionContext)(context, sett, caps);
	const auto level = sett->level;
	for (int i = 0; i < total; i++)
	{
		if (const auto currentEntry = entries[i]; currentEntry)
		{
			if (!GAME_FUNC(bool(__thiscall*)(DWORD, edbEntry*, bool), isBuildingTypePresent)(context, currentEntry, false)
				&& currentEntry->classification != 5
				&& (!currentEntry->isCoreBuilding || GAME_FUNC(bool(__thiscall*)(settlementStruct*), canSettlementUpgrade)(sett))
				&& (forceTemple || !currentEntry->isTemple || !sett->hasReligiousBuilding))
			{
				const bool isCastle = sett->isCastle;
				if (const auto buildLvl = currentEntry->classification == 4 ? level : 0;
					GAME_FUNC(bool(__thiscall*)(edbEntry*, int8_t, int, int, bool), isLevelAvailableForLevel)(currentEntry, static_cast<int8_t>(facId), level, buildLvl, isCastle))
				{
					if (GAME_FUNC(bool(__thiscall*)(edbEntry*, DWORD, int), evaluateBuildingGuard)(currentEntry, context, 0) && currentEntry->levels->buildTime)
					{
						const auto item = GAME_FUNC(buildingInQueue*(__thiscall*)(void*), createConstructionOption)(data);
						if (currentEntry->classification == 4)
							GAME_FUNC(void(__thiscall*)(buildingInQueue*, settlementStruct*, edbEntry*, int, void*), createConvertSett)(item, sett, currentEntry, level, bonus);
						else
							GAME_FUNC(void(__thiscall*)(buildingInQueue*, settlementStruct*, edbEntry*, void*), createBuildingSett)(item, sett, currentEntry, bonus);
						if (checkQueue && GAME_FUNC(bool(__thiscall*)(DWORD, buildingInQueue*), buildQueueConflictTest)(reinterpret_cast<DWORD>(sett) + 0x330, item))
							GAME_FUNC(void(__thiscall*)(void*), removeBuildingSelection)(data);
					}
				}
			}
		}
	}
}

int patchesForGame::onConflictTest(const buildingsQueue* queue, int index)
{
	for (int i = 0; i < queue->buildingQueue.num; i++)
	{
		const auto item = queue->buildingQueue[i];
		const auto entry = item.edbEntry;
		const auto build = item.existsBuilding;
		if ( item.constructType == 2)
		{
			if (build && build->edbEntry && build->edbEntry->buildingID == index)
				return 1;
		}
		else if (entry && entry->buildingID == index)
		{
			return 1;
		}
	}
	return 0;
}

int patchesForGame::onOffMapModelThing(const int culture)
{
	if (culture > 6)
		return 0;
	return culture;
}

DWORD* patchesForGame::onSetKhakiText(DWORD* text)
{
	if (m2tweopOptions::isSetKhaki && text && (*text == 0xFF807761 || *text == 0xFF4B3C0A || *text == 0xFF4C4836))
		*text = m2tweopOptions::getColor();
	else if (text)
		*text = minHookFunctions::checkSet(*text);
	return text;
}

int patchesForGame::onAddBuildingCapsAfterConstruction(const settlementStruct* sett, int index)
{
	for (int i = 0; i < sett->buildingsQueueArray.buildingQueue.num; i++)
	{
		const auto item = sett->buildingsQueueArray.buildingQueue[i];
		const auto entry = item.edbEntry;
		const auto build = item.existsBuilding;
		if ( item.constructType == 2)
		{
			if (build && build->edbEntry && build->edbEntry->buildingID == index)
				return 3;
		}
		else if (entry && entry->buildingID == index)
		{
			return 1;
		}
	}
	return 0;
}

building* patchesForGame::onCheckBuildUpgrade(const settlementStruct* sett, const int buildingId)
{
	if (buildingId < 0)
		return nullptr;
	for (int i = 0; i < sett->buildingsNum; i++)
	{
		if (sett->buildings[i]->edbEntry->buildingID == buildingId)
			return sett->buildings[i];
	}
	return nullptr;
}

void patchesForGame::onAttachRegionSettlement(settlementStruct* sett, int regionId)
{
	sett->regionID = regionId;
	const auto map = stratMapHelpers::getStratMap();
	const auto region = &map->regions[regionId];
	if ((region->settlement && region->settlement != sett) || sett->isMinorSettlement || sett->minorSettlementIndex > 0)
		return;
	region->settlement = sett;
	GAME_FUNC(void(__thiscall*)(stratMap*, int, factionStruct*),
		setRegionFaction)(map, sett->regionID, sett->faction);
	for (int i = 0; i < region->resourcesNum; i++)
	{
		GAME_FUNC(void(__thiscall*)(resourceStruct*, settlementStruct*),
			attachResourceSett)(region->resources[i], sett);
	}
}

int patchesForGame::chooseCaptainUnit(armyStruct* army)
{
	if (const auto battle = battleHelpers::getBattleData(); !battle || battle->battleCampaignType == 2)
		return 0;
	
	int bestUnit = -1;
	int bestBackupUnit = -1;
	int bestMercUnit = -1;
	float bestUnitValue = 0.f;
	float bestUnitValueTotal = 0.f;
	float bestBackupUnitValue = 0.f;
	float bestMercUnitValue = 0.f;
	
	for (int i = 0; i < army->numOfUnits; ++i)
	{
		const auto un = army->units[i];
		if ((un->eduEntry->generalUnit || un->eduEntry->generalUnitUpgrade) && !un->isMercenary)
		{
			return i;
		}
		const auto value = un->eduEntry->aiUnitValue;
		const auto valuePerSoldier = un->eduEntry->aiUnitValuePerSoldier;
		if (!un->isMercenary && (!un->eduEntry->mount || un->eduEntry->mount->mountClass != mountClass::elephant))
		{
			if ((static_cast<float>(un->SoldierCountStrat) / static_cast<float>(un->SoldierCountMaxStrat)) > 0.5f)
			{
				if (valuePerSoldier > bestUnitValue || (FLOAT_EQUAL(valuePerSoldier, bestUnitValue) && value > bestUnitValueTotal))
				{
					bestUnitValue = valuePerSoldier;
					bestUnit = i;
					bestUnitValueTotal = value;
				}
			}
			if (value > bestBackupUnitValue)
			{
				bestBackupUnit = i;
				bestBackupUnitValue = value;
			}
		}
		else if (value > bestMercUnitValue)
		{
			bestMercUnitValue = value;
			bestMercUnit = i;
		}
	}
	
	if (bestUnitValue > 0)
		return bestUnit;
	if (bestBackupUnit > 0)
		return bestBackupUnit;
	if (bestMercUnitValue > 0)
		return bestMercUnit;
	return 0;
}

character* __fastcall patchesForGame::onHealCasualties(battleSideArmy* sideArmy)
{
	const auto army = sideArmy->stack;
	const auto namedChar = army->gen->characterRecord;
	if (!m2tweopOptions::getRandomizeHealing())
	{
		return army->gen;
	}
	const int surgery = std::clamp(namedChar->battleSurgery, 0, 10);
	const auto lostSoldierCount = sideArmy->lostSoldiersCount;
	const auto battleData = battleHelpers::getBattleData();
	battleArmy* bArmy = nullptr;
	for (int s = 0; s < battleData->sidesNum; ++s)
	{
		const auto side = &battleData->sides[s];
		if (bArmy = battleHelpers::findBattleArmyWithSide(side, army); bArmy != nullptr)
			break;
	}
	if (!bArmy)
		return army->gen;
	auto soldiersToHeal = lostSoldierCount - bArmy->takenPrisonerSum();
	const float surgeryRatio = (0.15f + (0.03f * static_cast<float>(surgery)));
	soldiersToHeal = static_cast<int>(ceil(static_cast<float>(soldiersToHeal) * surgeryRatio));
	std::vector<lostSoldier*> candidates;
	for (int i = 0; i < lostSoldierCount; ++i)
	{
		auto* lostSoldier = &sideArmy->lostSoldiers[i];
		if (!lostSoldier->trackedUnit.unit || !lostSoldier->soldier)
		{
			continue;
		}
		auto* soldier = lostSoldier->soldier;
		if (!soldier->isRecoverable || soldier->isOfficer || soldier->isOfficerCommanding || soldier->isGeneral) 
		{
			continue;
		}
		if (soldier->captor) 
		{ 
			soldier->thisUnit->recoverSoldier(soldier); 
			soldier->captor = nullptr; 
			continue; 
		}
		candidates.push_back(lostSoldier);
	}
	const int n = min<int>(soldiersToHeal, static_cast<int>(candidates.size()));
	for (int k = 0; k < n; ++k)
	{
		std::swap(candidates[k], candidates[gameHelpers::getRandomInt(k, static_cast<int>(candidates.size()) - 1)]);
		auto* soldier = candidates[k]->soldier;
		if (auto* bUnit = bArmy->findBattleUnit(soldier->thisUnit))
		{
			soldier->thisUnit->recoverSoldier(soldier);
			bUnit->soldiersHealed++; 
			bUnit->soldiersEnd++;
		}
	}
	return nullptr;
}


int patchesForGame::onGetTrueBuildingCapabilities(const int counter, const stackCapabilities* cap)
{
	if (counter < 32)
		return 0;
	if (cap->settlement && cap->settlement->name)
		gameHelpers::logStringGame("ERROR: Settlement: " + std::string(cap->settlement->name) + " has too many true building capabilities!");
	else
		gameHelpers::logStringGame("A settlement has too many true building capabilities!");
	return 1;
}

void patchesForGame::onCalculateSettlement(settlementStruct* sett)
{
	if(!sett->isMinorSettlement)
		return;
	balanceMinorSettStats(&sett->stats.settlementStats, sett);
}

eduEntry* patchesForGame::onCustomBattleCost(const int eduIndexOffset)
{
	const int eduIndex = eduIndexOffset / 249;
	return eopDu::getEduEntry(eduIndex);
}

int patchesForGame::onMarriageOption(const factionRecord* facRecord)
{
	if (m2tweopOptions::getEnableFamilyEventsForTeutonic())
		return 1;
	return facRecord->hasFamilyTree && !facRecord->teutonic;
}

int patchesForGame::onCalcUnitStatsWpn(const int weapon)
{
	return weapon * m2tweopOptions::weaponBonusModifier;
}

int patchesForGame::onCalcUnitValueArmour(const int armour)
{
	return armour * m2tweopOptions::armourBonusModifier * 4;
}

//Sally out fix, consider armies around settlement not defender
int* patchesForGame::onGetSupportingArmies(armyStruct* defender, armyStruct* attacker)
{
	if (!defender->siege || !attacker->settlement)
		return reinterpret_cast<int*>(defender->getCoordPair());
	
	const auto attSett = attacker->findInSettlement();
	if (const auto sett = defender->siege->getSiegedSettlement(); attSett && sett)
	{
		if (sett->xCoord == attSett->xCoord && sett->yCoord == attSett->yCoord)
			return reinterpret_cast<int*>(&sett->xCoord);
	}
	const auto attFort= attacker->findInFort();
	if (const auto fort = defender->siege->getSiegedFort(); attFort && fort)
	{
		if (fort->xCoord == attFort->xCoord && fort->yCoord == attFort->yCoord)
			return reinterpret_cast<int*>(&fort->xCoord);
	}
	
	return reinterpret_cast<int*>(defender->getCoordPair());
}

uint32_t makeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

uint32_t patchesForGame::onDrawBanner(const trackedArmy* army)
{
	const auto stack = army->army;
	if (stack->bannerSet)
	{
		return makeColor(stack->bannerRed, stack->bannerGreen, stack->bannerBlue, 0xFF);
	}
	if (stack->descrRebel && stack->faction->factionID == campaignHelpers::getCampaignData()->slaveFactionID)
	{
		if (const auto rebelFac = eopRebelFactionDb::getRebelFaction(stack->descrRebel->getName()); rebelFac && rebelFac->bannerSet)
		{
			return makeColor(rebelFac->bannerRed, rebelFac->bannerGreen, rebelFac->bannerBlue, 0xFF);
		}
	}
	const auto factionRec = stack->faction->factionRecord;
	return makeColor(factionRec->primary_colour_red, factionRec->primary_colour_green, factionRec->primary_colour_blue, 0xFF);
}

bannerData* patchesForGame::onGetRebelSymbol(const trackedArmy* army, bannerData* oldData)
{
	const auto stack = army->army;
	if (!stack->descrRebel)
		return oldData;
	const auto rebelFac = eopRebelFactionDb::getRebelFaction(stack->descrRebel->getName());
	if (!rebelFac)
		return oldData;
	if (rebelFac->bannerSymbolSet)
		return rebelFac->banner.get();
	return oldData;
}

void patchesForGame::balanceMinorSettStats(settlementStats* stats, const settlementStruct* sett)
{
	const auto modifiers = minorSettlementDb::getModifiers();
	if (const auto region = stratMapHelpers::getRegion(sett->regionID); region->factionOwner->factionID != sett->faction->factionID)
	{
		const auto facDip = campaignHelpers::getCampaignData()->getFactionDiplomacy(region->factionOwner->factionID, sett->faction->factionID);
		float modifierIncome = modifiers->incomeModifierBase;
		float modifierGrowth = modifiers->growthModifierBase;
		if (facDip->state == dipStance::alliance)
		{
			modifierIncome += modifiers->incomeModifierAllianceOffset;
			modifierGrowth += modifiers->growthModifierAllianceOffset;
		}
		else if (facDip->state < dipStance::war)
		{
			modifierIncome += modifiers->incomeModifierNeutralOffset;
			modifierGrowth += modifiers->growthModifierNeutralOffset;
		}
		if (facDip->hasTradeRights)
		{
			modifierIncome += modifiers->incomeModifierTradeRightsOffset;
			modifierGrowth += modifiers->growthModifierTradeRightsOffset;
		}
		stats->TradeIncome = static_cast<int>(round(stats->TradeIncome * modifierIncome));
		stats->FarmsIncome = static_cast<int>(round(stats->FarmsIncome * modifierIncome));
		stats->DevastationExpense = static_cast<int>(round(stats->DevastationExpense * modifierIncome));
		stats->MiningIncome = static_cast<int>(round(stats->MiningIncome * modifierIncome));
		stats->PopGrowthFarms = static_cast<int>(round(stats->PopGrowthFarms * modifierGrowth));
		stats->PopGrowthBaseFarm = static_cast<int>(round(stats->PopGrowthBaseFarm * modifierGrowth));
		stats->PopGrowthTrade = static_cast<int>(round(stats->PopGrowthTrade * modifierGrowth));
	}
	else
	{
		stats->TradeIncome = static_cast<int>(round(stats->TradeIncome * modifiers->incomeModifierOwnFaction));
		stats->FarmsIncome = static_cast<int>(round(stats->FarmsIncome * modifiers->incomeModifierOwnFaction));
		stats->DevastationExpense = static_cast<int>(round(stats->DevastationExpense * modifiers->incomeModifierOwnFaction));
		stats->MiningIncome = static_cast<int>(round(stats->MiningIncome * modifiers->incomeModifierOwnFaction));
		stats->PopGrowthFarms = static_cast<int>(round(stats->PopGrowthFarms * modifiers->growthModifierOwnFaction));
		stats->PopGrowthBaseFarm = static_cast<int>(round(stats->PopGrowthBaseFarm * modifiers->growthModifierOwnFaction));
		stats->PopGrowthTrade = static_cast<int>(round(stats->PopGrowthTrade * modifiers->growthModifierOwnFaction));
	}
	stats->TotalIncomeWithoutAdmin = stats->TradeIncome + stats->FarmsIncome + stats->MiningIncome + stats->TaxesIncome;
}

void patchesForGame::onPredictedStats(settlementStats* statsManager)
{
	if(!statsManager->settlement->isMinorSettlement)
		return;
	balanceMinorSettStats(statsManager, statsManager->settlement);
}

void* __fastcall patchesForGame::onBattleCompleted()
{
	gameEvents::onBattleCompleted();
	return campaignHelpers::getStratPathFinding();
}

int patchesForGame::onEvalAttObjective(const aiCampaignController* controller)
{
	int factionId = controller->regionData->getTargetFaction()->factionID;
	if (factionId != controller->regionData->factionID || !controller->attacking)
		return factionId;
	factionId = controller->targetFactionId;
	if (factionId == controller->regionData->factionID && controller->attacking)
		factionId = campaignHelpers::getCampaignData()->slaveFactionID;
	return factionId;
}

void patchesForGame::onCalculateLTGD(aiLongTermGoalDirector* ltgd)
{
	ltgd->update();
}

void patchesForGame::onStartProductionTurn(aiPersonalityValues* personality)
{
	personality->init();
}

factionStruct* patchesForGame::onCheckGarrison(const aiRegionController* controller)
{
	return controller->settlement->faction;
}

int patchesForGame::onValidateGarrison(const aiRegionController* controller, const armyStruct* army)
{
	return army->settlement == controller->settlement ? 1 : 0;
}

void patchesForGame::onUpdateControllerAlloc(aiCampaignController* controller)
{
	controller->updateAllocation();
}

int patchesForGame::onScoreBestCapital(const settlementStruct* sett)
{
	if (sett->isMinorSettlement)
		return 0;
	return 1;
}

std::string CARD_PATH;

int patchesForGame::onGetUnitCard(const eduEntry* entry, const int factionId, stringWithHash* newPath)
{
	if (!m2tweopOptions::getHandleUnitCards())
		return 0;
	const auto facRecord = factionHelpers::getFactionRecord(factionId);
	if (!facRecord || !entry->unitCardTga || !facRecord->facName)
		return 0;
	const string facName = facRecord->facName;
	const string modPath = gameHelpers::getModPath();
	CARD_PATH = modPath + "/data/ui/units/" + facName + "/" + entry->unitCardTga;
	if (std::filesystem::exists(CARD_PATH))
	{
		gameStringHelpers::setHashedString(&newPath->name, CARD_PATH.c_str());
		return 1;
	}
	return 0;
}

int patchesForGame::onGetUnitEduIndex(const stringWithHash* entryName)
{
	if (!entryName || !entryName->name)
		return -1;
	return unitHelpers::getEduIndex(entryName->name);
}

int patchesForGame::onGetUnitInfoCard(const eduEntry* entry, const int factionId, stringWithHash* newPath)
{
	if (!m2tweopOptions::getHandleUnitCards())
		return 0;
	const auto facRecord = factionHelpers::getFactionRecord(factionId);
	if (!facRecord || !entry->infoCardTga || !facRecord->facName)
		return 0;
	const string facName = facRecord->facName;
	const string modPath = gameHelpers::getModPath();
	CARD_PATH = modPath + "/data/ui/unit_info/" + facName + "/" + entry->infoCardTga;
	if (std::filesystem::exists(CARD_PATH))
	{
		gameStringHelpers::setHashedString(&newPath->name, CARD_PATH.c_str());
		return 1;
	}
	return 0;
}

int patchesForGame::onAssessRequiredStrength(const aiRegionController* controller)
{
	if (!controller)
		return 0;
	return 1;
}

int patchesForGame::onCalcGarrisonStr(const aiRegionData* regData, const factionStruct* fac,
	const settlementStruct* sett)
{
	return campaignAi::assessGarrisonStrength(regData, sett, fac);
}

int __fastcall patchesForGame::onAttackGate(unit* un, void* tactic)
{
	const auto eduEntry = un->eduEntry;
	if (eduEntry == nullptr)
		return 0;
	if (eduEntry->mountedEngine)
	{
		const auto building = *reinterpret_cast<buildingBattle**>(reinterpret_cast<DWORD>(tactic) + 0x2AD0);
		if (building == nullptr)
			return 0;
		unitActions::attackBuilding(un, building);
		return 1;
	}
	return 0;
}

eduEntry* __fastcall patchesForGame::onCreateMercUnit(char** entryName, eduEntry* entry)
{
	return unitHelpers::getEduEntryByName(*entryName);
}

eduEntry* __fastcall patchesForGame::onCreateUnitWrapper(int eduIndexBase, int removeValue)
{
	const int eduIndex = eduIndexBase - (removeValue * 8);
	eduEntry* entry = eopDu::getEduEntry(eduIndex);
	return entry;
}

DWORD __fastcall patchesForGame::onUnitInfo(DWORD entryAddress)
{
	DWORD eduBaseAddress = dataOffsets::offsets.unitTypesStart;
	int index = (entryAddress - eduBaseAddress) / 996; // 996 is the size of the eduEntry struct
	if (index < 500)
	{
		return entryAddress;
	}
	auto entry = eopDu::getEopEduEntry(index);
	return reinterpret_cast<DWORD>(entry);
}

eduEntry* __fastcall patchesForGame::onGetRecruitPoolUnitEntry(int eduIndex)
{
	return eopDu::getEduEntry(eduIndex);
}

DWORD getVFunc(DWORD* addr, DWORD offset)
{
	DWORD vtbl = reinterpret_cast<DWORD>(addr);
	return *reinterpret_cast<DWORD*>(vtbl + offset);
}

enum class groupMoveType
{
	formedStatic,
	formed,						
	unformedUnitsFormed,		
	unformedUnitsUnformed,	
};

float distance(const float x, const float y, const float x2, const float y2)
{
	return static_cast<float>(sqrt(pow(x - x2, 2) + pow(y - y2, 2)));
}

bool USE_NEW_PLACEMENT = true;

void newPlacement(aiTacticAssault* aiTactic)
{
	const auto battleData = battleHelpers::getBattleData(); 
	if (!battleData || battleData->battleState >= 5)
		return;
	const DWORD vFunc = getVFunc(aiTactic->vftable, 0x4C);
	if (const int tacticType = GAME_FUNC_RAW(int(__thiscall*)(aiTacticAssault*), vFunc)(aiTactic); tacticType != 19)
		return;
	const auto battleSideArmy = battleData->getSideArmy(aiTactic->units[0]->army);
	const auto area = &battleSideArmy->deploymentAreas.get(0)->area;
	const auto unitDeployer = techFuncs::createGameClass<unitGroupDeployer>();
	GAME_FUNC(void(__thiscall*)(unitGroupDeployer*, unitGroup*, deploymentAreaS*), createUnitGroupDeployer)(unitDeployer, &aiTactic->aiUnitGroup, area);
	vector2 pos{};
	pos.x = aiTactic->advanceX;
	pos.y = aiTactic->advanceY;
	const bool found = GAME_FUNC(bool(__thiscall*)(unitGroupDeployer*, vector2*, int16_t, vector2*, bool, bool), findDeployPosition)(unitDeployer, &pos, aiTactic->angle, &pos, true, false);
	GAME_FUNC(void(__thiscall*)(unitGroup*, vector2*, int16_t, int, int), placeGroup)(&aiTactic->aiUnitGroup, &pos, aiTactic->angle, 1, 1);
	GAME_FUNC(void(__thiscall*)(unitGroup*, vector2*, int16_t, int, int), placeGroup)(&aiTactic->siegeUnitGroup, &pos, aiTactic->angle, 1, 1);
}

void __fastcall patchesForGame::onPreBattlePlacement(aiTacticAssault* aiTactic)
{
	if (USE_NEW_PLACEMENT)
	{
		newPlacement(aiTactic);
	}
	else
	{
		constexpr float extraDist = 70.0;
		if (const auto battle = battleHelpers::getBattleData(); !battle || battle->battleState >= 5)
			return;
		const DWORD vFunc = getVFunc(aiTactic->vftable, 0x4C);
		if (const int tacticType = GAME_FUNC_RAW(int(__thiscall*)(aiTacticAssault*), vFunc)(aiTactic); tacticType != 19)
			return;
		const auto group1 = &aiTactic->aiUnitGroup;
		const auto group2 = &aiTactic->siegeUnitGroup;
		struct battleCoords
		{
			float x;
			float y;
		};
		const auto coords = new battleCoords;
		const auto facingX = 0.0f - battleHelpers::getSineValues()[aiTactic->angle >> 2];
		const auto facingY = 0.0f - battleHelpers::getSineValues()[(aiTactic->angle + 0x4000) >> 2];
		coords->x = aiTactic->advanceX + (facingX * extraDist);
		coords->y = aiTactic->advanceY + (facingY * extraDist);
		const DWORD funcAddr = codes::offsets.issueMoveOrder;
		GAME_FUNC_RAW(bool(__thiscall*)(aiUnitGroup*, float*, int16_t, int, bool, bool), funcAddr)(
			group1,
			&coords->x,
			aiTactic->angle,
			static_cast<int>(groupMoveType::formed),
			true,
			true);
		for (int i = 0; i < group1->unitsInFormationNum; i++)
		{
			if (const auto unit = group1->unitsInFormation[i]; unit && distance(unit->positionX, unit->positionY, coords->x, coords->y) > 500)
				unitActions::placeUnit(unit, coords->x, coords->y, aiTactic->angle, 0);
		}
		GAME_FUNC_RAW(bool(__thiscall*)(aiUnitGroup*, float*, int16_t, int, bool, bool), funcAddr)(
			group2,
			&coords->x,
			aiTactic->angle,
			static_cast<int>(groupMoveType::formed),
			true,
			true);
		delete coords;
	}
}

std::shared_ptr<std::vector<int>> USED_GATES = std::make_shared<std::vector<int>>();
aiDetachment* LAST_DETACHMENT = nullptr;
int GATES_IGNORED = 0;

bool __fastcall patchesForGame::onDecideRamAttacks(buildingBattle* gate, aiDetachment* detachment, int numRamsLeft)
{
	if (gate->perimeter > 0 && battleHelpers::getBattleData()->battleState >= 5)
	{
		return true;
	}
	if (detachment != LAST_DETACHMENT)
	{
		GATES_IGNORED = 0;
		LAST_DETACHMENT = detachment;
	}
	if (numRamsLeft + GATES_IGNORED <= 0)
		return false;
	for (auto usedGate : *USED_GATES)
	{
		if (gate->buildingIndex == usedGate)
		{
			GATES_IGNORED++;
			gameHelpers::logStringGame("Gate already used: " + std::to_string(usedGate));
			return false;
		}
	}
	int numLaddersTowers = 0;
	int numRams = 0;
	int infCavNum = 0;
	for (int i = 0; i < detachment->aiDetachUnitsCount; i++)
	{
		auto unit = detachment->aiDetachUnits[i].unit;
		if (unit->eduEntry->category != unitCategory::siege
			&& unit->eduEntry->unitClass != unitClass::missile
			&& unit->eduEntry->unitClass != unitClass::skirmish)
			infCavNum++;
		if (unit->eduEntry->category != unitCategory::infantry)
			continue;
		if (unit->siegeEnNum == 0)
			continue;
		auto engine = unit->siegeEngines[0];
		if (engine == nullptr)
			continue;
		if (engine->engineRec->classID == engineType::ram)
			numRams++;
		if (engine->engineRec->classID == engineType::ladder || engine->engineRec->classID == engineType::tower)
			numLaddersTowers++;
	}
	auto battleData = battleHelpers::getBattleData();
	battleSide* attacker = nullptr;
	battleSide* defender = nullptr;
	for (int i = 0; i < battleData->sidesNum; i++)
	{
		auto side = battleData->sides[i];
		if (side.isDefender)
			defender = &side;
		else
			attacker = &side;
	}
	auto defenderArmy = defender->armies[0].stack;
	int infCavNumDef = 0;
	if (defenderArmy)
	{
		for (int i = 0; i < defenderArmy->numOfUnits; i++)
		{
			if (auto unit = defenderArmy->units[i];
				unit->eduEntry->category != unitCategory::siege
			&& unit->eduEntry->unitClass != unitClass::missile
			&& unit->eduEntry->unitClass != unitClass::skirmish)
				infCavNumDef++;
		}
	}
	auto battleSideArmy = battleData->getSideArmy(detachment->aiDetachUnits[0].unit->army);
	auto deployArea = &battleSideArmy->deploymentAreas.get(0)->area;
	auto areaX = deployArea->centreX;
	auto areaY = deployArea->centreY;
	int bonus = std::clamp(infCavNum - infCavNumDef, -3, 3);
	int ramsNeeded = (infCavNum - numLaddersTowers) / (4 - bonus);
	std::vector<buildingBattle*> gates{};
	auto buildings = gate->battleResidence->battleBuildings;
	for (int i = 0; i < buildings->allBuildingsNum; i++)
	{
		if (auto building = buildings->allBuildings[i]; building->type == 3)
			gates.push_back(building);
	}
	std::sort(gates.begin(), gates.end(), [areaX, areaY](buildingBattle* a, buildingBattle* b)
		{
			return distance(a->xCoord, a->yCoord, areaX, areaY) < distance(b->xCoord, b->yCoord, areaX, areaY);
		});
	if (gate == gates[0])
	{
		gameHelpers::logStringGame("Using gate 0: " + std::to_string(gate->buildingIndex) + " with detachment " + std::to_string(reinterpret_cast<DWORD>(detachment)));
		USED_GATES->push_back(gate->buildingIndex);
		return true;
	}
	int gateNum = static_cast<int>(gates.size());
	ramsNeeded = std::clamp(ramsNeeded, 1, gateNum);
	if (int ramsUsed = numRams - numRamsLeft; ramsUsed >= ramsNeeded)
		return false;
	if (ramsNeeded < gateNum)
	{
		for (int i = 0; i < gateNum - ramsNeeded; i++)
			gates.pop_back();
	}
	bool isCloseEnough = false;
	for (int i = 0; i < deployArea->coordsNum; i++)
	{
		if (distance(deployArea->coordsPairs[i].xCoord, deployArea->coordsPairs[i].yCoord, gate->xCoord, gate->yCoord) < 250)
			isCloseEnough = true;
		if (!gates.empty() && distance(gates[0]->xCoord, gates[0]->yCoord, gate->xCoord, gate->yCoord) < 200)
			isCloseEnough = true;
		if (isCloseEnough)
			break;
	}
	if (!isCloseEnough)
		return false;
	if (std::any_of(gates.begin(), gates.end(), [gate](const buildingBattle* gt){return gate == gt;}))
	{
		gameHelpers::logStringGame("Using gate: " + std::to_string(gate->buildingIndex) + " with detachment " + std::to_string(reinterpret_cast<DWORD>(detachment)));
		USED_GATES->push_back(gate->buildingIndex);
		return true;
	}
	return false;
}

bool __thiscall patchesForGame::onPreBattlePlacement2(aiUnitGroup* group, DWORD formationTemplate, bool forceOrder)
{
	DWORD orderChangeFormation = codes::offsets.issueFormationOrder;
	auto retBool = GAME_FUNC_RAW(bool(__thiscall*)(aiUnitGroup*, DWORD, bool), orderChangeFormation)(group, formationTemplate, forceOrder);
	auto battle = battleHelpers::getBattleData();
	if (battle->battleType != 3)
		return retBool;
	if (battle->battleState >= 5)
		return retBool;
	auto tactic = group->detachmentTactic;
	if (!tactic)
		return retBool;
	DWORD vFunc = getVFunc(tactic->vftable, 0x4C);
	int tacticType = GAME_FUNC_RAW(int(__thiscall*)(aiDetachmentTactic*), vFunc)(tactic);
	if (tacticType != 19)
		return retBool;
	auto aiTactic = reinterpret_cast<aiTacticAssault*>(tactic);
	float posX = aiTactic->building->xCoord;
	float posY = aiTactic->building->yCoord;
	aiTactic->advanceX = posX;
	aiTactic->advanceY = posY;
	auto group1 = aiTactic->aiUnitGroup;
	auto angle = unitHelpers::angleShortToFloat(aiTactic->angle);
	for (int i = 0; i < group1.unitsInFormationNum; i++)
	{
		auto unit = group1.unitsInFormation[i];
		unitActions::placeUnit(unit, posX, posY, aiTactic->angle, 0);
		gameHelpers::logStringGame("Unit placed at2: " + to_string(unit->positionX) + " " + to_string(unit->positionY) + " " + to_string(angle));
		group1.xCoord = unit->positionX;
		group1.yCoord = unit->positionY;
		group1.angle = aiTactic->angle;
	}
	auto group2 = aiTactic->siegeUnitGroup;
	for (int i = 0; i < group2.unitsInFormationNum; i++)
	{
		auto unit = group2.unitsInFormation[i];
		unitActions::placeUnit(unit, posX, posY, aiTactic->angle, 0);
		group2.xCoord = unit->positionX;
		group2.yCoord = unit->positionY;
		group2.angle = aiTactic->angle;
		gameHelpers::logStringGame("Siege unit placed at2: " + to_string(unit->positionX) + " " + to_string(unit->positionY) + " " + to_string(angle));
	}
	return retBool;
}

const char* __fastcall patchesForGame::onQuickSave()
{
	static std::vector<std::string> saveNames = { u8"%S-1.sav" ,u8"%S-2.sav", u8"%S-3.sav" };
	jsn::json json;

	std::string fPath = globals::dataS.modPath;
	fPath += "\\saves\\quickSavesM2TWEOP.json";

	int currSaveID = 0;
	int maxSaveID = 2;
	try
	{
		std::ifstream f1(fPath);
		if (f1.is_open())
		{
			f1 >> json;

			f1.close();

			json.at("saveID").get_to(currSaveID);

			if (currSaveID + 1 > maxSaveID)
			{
				json["saveID"] = 0;
			}
			else
			{
				json["saveID"] = currSaveID + 1;
			}
			std::ofstream f2(fPath);

			f2 << json;

			f2.close();
		}
		else
		{
			currSaveID = 0;
			json["saveID"] = 0;

			std::ofstream f2(fPath);

			f2 << json;

			f2.close();
		}
	}
	catch (jsn::json::type_error& e)
	{
		currSaveID = 0;
		json["saveID"] = 0;
		std::ofstream f2(fPath);
		f2 << json;
		f2.close();
		gameHelpers::logStringGame("Error in quickSave " + std::string(e.what()));
	}

	return saveNames[currSaveID].c_str();
}
const char* __fastcall patchesForGame::onAutoSave()
{
	static std::vector<std::string> saveNames = { u8"%s%S%s%S-1.sav" ,u8"%s%S%s%S-2.sav", u8"%s%S%s%S-3.sav",
		 u8"%s%S%s%S-4.sav" ,u8"%s%S%s%S-5.sav", u8"%s%S%s%S-6.sav"
	};
	jsn::json json;

	std::string fPath = globals::dataS.modPath;
	fPath += "\\saves\\autoSavesM2TWEOP.json";

	int currSaveID = 0;
	int maxSaveID = 5;
	try
	{
		std::ifstream f1(fPath);
		if (f1.is_open())
		{
			f1 >> json;

			f1.close();

			json.at("saveID").get_to(currSaveID);

			if (currSaveID + 1 > maxSaveID)
			{
				json["saveID"] = 0;
			}
			else
			{
				json["saveID"] = currSaveID + 1;
			}
			std::ofstream f2(fPath);

			f2 << json;

			f2.close();
		}
		else
		{
			currSaveID = 0;
			json["saveID"] = 0;

			std::ofstream f2(fPath);

			f2 << json;

			f2.close();
		}
	}
	catch (jsn::json::type_error& e)
	{
		currSaveID = 0;
		json["saveID"] = 0;
		std::ofstream f2(fPath);
		f2 << json;
		f2.close();
		gameHelpers::logStringGame("Error in autoSave " + std::string(e.what()));
	}

	return saveNames[currSaveID].c_str();
}
character* __fastcall patchesForGame::mercenaryMovePointsGetGeneral(armyStruct* army)
{
	character* gen = army->gen;
	if (gen == nullptr)
	{
		if (army->settlement != nullptr)
		{
			const auto ourFort = reinterpret_cast<fortStruct*>(army->settlement);
			const auto nextObject = ourFort->nextObject;
			if (nextObject == nullptr)
				return nullptr;
			if (const int objectType = callVFunc<4, int>(nextObject); objectType == 4)
			{
				gen = static_cast<character*>(nextObject);
			}
		}
	}
	return gen;
}

void __fastcall patchesForGame::clickAtTile(coordPair* xy)
{
	const auto tile = stratMapHelpers::getTile(xy->xCoord, xy->yCoord);
	if (!tile)
		return;
	if (const auto character = tile->getCharacter(); character)
		onCharacterClicked(character);
	gameEvents::onClickAtTile(xy->xCoord, xy->yCoord);
	plannedRetreatRoute::onClickAtTile(xy->xCoord, xy->yCoord);

	minHookFunctions::selectTile = tile;
}

void __fastcall patchesForGame::onCharacterClicked(character* enemy)
{
	gameEvents::onCharacterClicked(enemy);
}

DWORD patchesForGame::onCampaignTick()
{
	gameEvents::onCampaignTick();
	const auto stratMap = stratMapHelpers::getStratMap();
	return reinterpret_cast<DWORD>(stratMap);
}

void patchesForGame::onBattleTick()
{
	gameEvents::onBattleTick();
}

bool NEED_BUILD_FRONTIERS = true;
bool LOADING_SAVE = false;

void __stdcall patchesForGame::afterCampaignMapLoaded()
{
	globals::dataS.Modules.tacticalMapViewer.unView();
	plugData::data.luaAll.fillHashMaps();
	gameEvents::onCampaignMapLoaded();
	eopCharacterDataDb::get()->onGameLoaded();
	if (NEED_BUILD_FRONTIERS && m2tweopOptions::getUseEopFrontiers())
	{
		stratMapHelpers::rebuildFrontiers();
		NEED_BUILD_FRONTIERS = false;
	}
	discordManager::onCampaignMapLoaded();
	if (LOADING_SAVE)
	{
		eopFactionDataDb::get()->onGameLoaded();
		LOADING_SAVE = false;
	}
}

void __stdcall patchesForGame::onNewGameStart()
{
	minorSettlementDb::clear();
	eopSettlementDataDb::get()->clearData();
	gameStringDataDb::getInstance()->restoreOriginal();
	gameEvents::onNewGameStart();
	settlementHelpers::clearOptionsDb();
	plannedRetreatRoute::onNewGameStart();
}

//#define TESTPATCHES
void __stdcall patchesForGame::onEduParsed()
{
	gameHelpers::fixReligionTrigger();
	gameHelpers::fixRemoveSiegeLoop();
	*reinterpret_cast<bool*>(dataOffsets::offsets.bugReport) = true;
	unitHelpers::initBaseUnitsLookup();
	gameEvents::onReadGameDbsAtStart();
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "afterEDUread" << endl;
	f1.close();
#endif
}

namespace
{
	bool FIRST_LOAD = true;
}

void __stdcall patchesForGame::onGameInit()
{
	gameHelpers::logStringGame("onGameInit");
	plugData::data.luaAll.fillHashMapsNonCampaign();
	plugData::data.luaAll.fillHashMapsVnv();
	cultures::eopPortraitDb::createEopPortraitDb();
	discordManager::menuLoaded();
	eopFactionDataDb::get()->getOriginalData();
	gameEvents::onGameInit();
	*reinterpret_cast<bool*>(dataOffsets::offsets.bugReport) = true;
	eopDu::fixCustomBattleGeneralEntries();
	//gameHelpers::getOptions1()->microManageAllSettlements = 1;
	gameHelpers::setNoAiLoose(m2tweopOptions::getNoAiMissileLooseFormation());
	gameHelpers::setDisplayEnemyStats(m2tweopOptions::getDisplayEnemyStats());
}

eopCamera stratCamera;
void __stdcall patchesForGame::onUnloadCampaign()
{
	eopRebelFactionDb::clear();
	globalEopAiConfig::clearFactionData();
	discordManager::menuLoaded();
	gameEvents::onUnloadCampaign();
	stratCamera.isInit = false;
}
void __stdcall patchesForGame::onNewGameLoaded()
{
	if (!eopHiddenResources::isInitialized())
		eopHiddenResources::initialize();
	const auto sMap = stratMapHelpers::getStratMap();
	sMap->fixAllReligionLevels();
	eopFactionDataDb::get()->restoreOriginalData();
	eopCharacterDataDb::get()->clearData();
	minorSettlementDb::load();
	eopSettlementDataDb::get()->newGameLoaded();
	eopRebelFactionDb::loadData();
	plugData::data.luaAll.fillHashMaps();
	gameEvents::onNewGameLoaded();
	NEED_BUILD_FRONTIERS = true;
}

bool AI_ACTIVE = false;
void __fastcall patchesForGame::onAiTurn(aiFaction* aiFac)
{
	gameEvents::onAiTurn(aiFac);
}

void __stdcall patchesForGame::onChangeTurnNum()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "onChangeTurnNum" << endl;
	f1.close();
#endif
	eopCharacterDataDb::validate();
	const int num = campaignHelpers::getCampaignData()->turnNumber;
	discordManager::onChangeTurnNum(num);
	gameEvents::onChangeTurnNum(num);
}

void __stdcall patchesForGame::onGiveTrait()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "onGiveTrait" << endl;
	f1.close();
#endif
}

void __stdcall patchesForGame::afterLoadGameData()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "afterLoadGameData" << endl;
	f1.close();
#endif
}

void __stdcall patchesForGame::afterI_CompareCounter()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "afterI_CompareCounter" << endl;
	f1.close();
#endif
}

int __stdcall patchesForGame::checkFort()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "checkFort" << endl;
	f1.close();
#endif
	return 0;
}

int __stdcall patchesForGame::checkDipString()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "checkDipString" << endl;
	f1.close();
#endif
	return 0;
}

int __stdcall patchesForGame::checkFacDip()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "checkFacDip" << endl;
	f1.close();
#endif
	return 0;
}

int __stdcall patchesForGame::spawnArmyCoords()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "spawnArmyCoords" << endl;
	f1.close();
#endif
	return 0;
}

void __stdcall patchesForGame::spawnEndArmyCoords()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "spawnEndArmyCoords" << endl;
	f1.close();
#endif
}

int __stdcall patchesForGame::spawnCharacterCoords()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "spawnCharacterCoords" << endl;
	f1.close();
#endif
	return 0;
}

void __stdcall patchesForGame::onBattleStratScreen()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "onBattleStratScreen" << endl;
	f1.close();
#endif
	//battleCreator::onBattleStratScreen();

}

void deployGateAttackers(const aiTacticAssault* aiTactic)
{
	if (const auto unit = aiTactic->attackUnit; unit && distance(unit->positionX, unit->positionY, aiTactic->advanceX, aiTactic->advanceY) > 500)
		unitActions::placeUnit(unit, aiTactic->advanceX, aiTactic->advanceY, aiTactic->angle, 0);
}

bool FIRST_END = true;
bool NEED_BUILD_FRONTIER = true;

void __fastcall patchesForGame::onEvent(DWORD** vTab, DWORD arg2)
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "onEvent" << endl;
	f1.close();
#endif

	const DWORD eventCode = reinterpret_cast<DWORD>(vTab[0]);

	const int gameVersion = gameHelpers::getGameVersion();
	const DWORD scrollOpenedCode = gameVersion == 1 ? 0x013719FC : 0x0132C9D4;
	const DWORD scrollClosedCode = gameVersion == 1 ? 0x01371AB4 : 0x0132CA8C;
	const DWORD factionTurnStartCode = gameVersion == 1 ? 0x0136931C : 0x013242F4;
	const DWORD preFactionTurnStartCode = gameVersion == 1 ? 0x013693EC : 0x013243C4;
	const DWORD gameReloaded = gameVersion == 1 ? 0x013319E4 : 0x012EC9C4;
	const DWORD settlementTurnStart = gameVersion == 1 ? 0x0136E2B4 : 0x0132928C;
	const DWORD settlementTurnEnd = gameVersion == 1 ? 0x0136FADC : 0x0132AAB4;
	const DWORD conflictPhaseCommenced = gameVersion == 1 ? 0x013670AC : 0x01322084 ;
	const DWORD factionTurnEnd = gameVersion == 1 ? 0x01369D74 : 0x01324D4C;
	const DWORD characterTurnEnd = gameVersion == 1 ? 0x0136C0B4 : 0x0132708C;
	const DWORD characterTurnStart = gameVersion == 1 ? 0x0136BFE4 : 0x01326FBC;
	const DWORD postBattle = gameVersion == 1 ? 0x01367ADC : 0x01322AB4; 
	const DWORD preBattleWithdrawal = gameVersion == 1 ? 0x01366D54 : 0x01321D2C; 
	const DWORD generalAssaultsResidence = gameVersion == 1 ? 0x0136A3E4 : 0x013253BC; 
	const DWORD deploymentPhaseCommenced = gameVersion == 1 ? 0x01366FD4 : 0x01321FAC; 
	const DWORD buttonPressedCode = gameVersion == 1 ? 0x013717D4 : 0x0132C7AC;
	if (eventCode == scrollOpenedCode)
	{
		char* str = reinterpret_cast<char*>(vTab[1]);
		if (strcmp(str, "prebattle_scroll") == 0)
		{
			battleCreator::onBattleStratScreen();
			campaignAi::setPlayerAssaulted();
			stratCamera.isScrollOpen = true;
		}
		else if (strcmp(str, "post_battle_scroll") == 0)
		{
			battleCreator::onPostBattleStratScreen();
			const auto campData = campaignHelpers::getCampaignData();
			if (campData->type == 3)
				minorSettlementDb::clear();
		}
		else if (strcmp(str, "hotseat_scroll") == 0)
		{
			battleCreator::onHotseatScreen();
		}
		else if (strcmp(str, "hotseat_scroll") == 0)
		{
			battleCreator::onHotseatScreen();
		}
		/*
		else if (m2tweopOptions::getHideUnknownUnitTooltips() && !battleHelpers::inBattle())
		{
			if (strcmp(str, "enemy_army_info_scroll") == 0)
			{
				gameHelpers::logStringGame("enemy_army_info_scroll");
				gameUiHelpers::removeToolTipsArmy();
			}
			else if (strcmp(str, "enemy_character_info_scroll") == 0)
			{
				gameHelpers::logStringGame("enemy_character_info_scroll");
				gameUiHelpers::removeToolTipsArmy();
			}
			else if (strcmp(str, "enemy_settlement_info_scroll") == 0)
			{
				gameHelpers::logStringGame("enemy_settlement_info_scroll");
				gameUiHelpers::removeToolTipsSett();
			}
		}*/
		else if (strcmp(str, "own_named_character_info_scroll") == 0 || strcmp(str, "fort_info_scroll") == 0)
		{
			minHookFunctions::isMercenaryScrollOpen = true;
		}
	}
	else if (eventCode == scrollClosedCode)
	{
		char* str = reinterpret_cast<char*>(vTab[1]);
		if (strcmp(str, "own_named_character_info_scroll") == 0 || strcmp(str, "fort_info_scroll") == 0)
		{
			minHookFunctions::resetMercScrollCache();
		}
	}
	else if (eventCode == factionTurnStartCode)
	{
		minHookFunctions::resetMercScrollCache();
		factionStruct* fac = reinterpret_cast<factionStruct*>(vTab[1]);
		gameHelpers::logStringGame("Faction turn start: " + string(fac->factionRecord->facName));
		globalEopAiConfig::getInstance()->turnStartMove(fac, false);
		discordManager::onFactionTurnStart(fac);
		plannedRetreatRoute::onFactionTurnStart(fac);
		if (fac->factionID == campaignHelpers::getCampaignData()->getFactionByOrder(0)->factionID)
		{
			eopCharacterDataDb::get()->reapplyPortraits();
		}
		if (fac->isPlayerControlled == 1)
		{
			for (int i = 0; i < fac->settlementsNum; i++)
			{
				if (const auto sett = fac->settlements[i];
					sett->aiProductionController)
				{
					sett->aiProductionController->isAutoManagedTaxes = false;
				}
			}
		}
		AI_ACTIVE = true;
	}
	else if (eventCode == preFactionTurnStartCode)
	{
		factionStruct* fac = reinterpret_cast<factionStruct*>(vTab[1]);
		const auto campaign = campaignHelpers::getCampaignData();
		//This fixes some bugs with using the control console command, like winning battles if you quit them.
		if (fac->isPlayerControlled == 1)
			campaign->playerFacBitMap |= (1 << fac->factionID);
		else
			campaign->playerFacBitMap &= ~(1 << fac->factionID);
		
	}
	else if (eventCode == gameReloaded)
	{
		minHookFunctions::resetMercScrollCache();
		if (!eopHiddenResources::isInitialized())
			eopHiddenResources::initialize();
		minorSettlementDb::load();
		eopSettlementDataDb::get()->onGameLoaded();
		eopFortDataDb::get()->onGameLoaded();
		eopFactionDataDb::get()->onGameLoaded();
		//eopCharacterDataDb::get()->onGameLoaded();
	}
	else if (eventCode == deploymentPhaseCommenced)
	{
		minHookFunctions::resetMercScrollCache();
	}
	else if (eventCode == conflictPhaseCommenced)
	{
		const auto battle = battleHelpers::getBattleData();
		gameHelpers::logStringGame("conflict phase commenced");
		USED_GATES->clear();
		if (battle && battle->battleType == battleType::siege)
		{
			if (const auto aiSide = battle->getAiSide(); aiSide && !aiSide->isDefender)
			{
				if (const auto battleAi = aiSide->battleAIPlan; battleAi->objectivesCount > 0)
				{
					for (int i = 0; i < battleAi->objectivesCount; i++)
					{
						const auto objective = battleAi->objectives[i];
						if (const auto objType = objective->getType(); objType == gtaObjective_ATTACK_SETTLEMENT)
						{
							for (int j = 0; j < objective->aiDetachmentsCount; j++)
							{
								const auto detachment = objective->aiDetachments[j];
								if (detachment->aiDetachUnitsCount == 0)
									continue;
								for (int t = 0; t < detachment->tacticsCount; t++)
								{
									const auto tactic = detachment->tactics[t];
									if (const auto tacticType = callClassFunc<aiDetachmentTactic*, int>(tactic, 0x4C); tacticType == 19)
									{
										const auto assaultTactic = reinterpret_cast<aiTacticAssault*>(tactic);
										deployGateAttackers(assaultTactic);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else if (eventCode == characterTurnEnd)
	{
		/*
		if (const auto record = reinterpret_cast<characterRecord*>(vTab[1]);
			record->gen
			&& record->faction->isPlayerControlled == 0
			&& record->gen->isGeneral()
			&& record->gen->army
			)
		{
			globalEopAiConfig::getInstance()->characterTurnStart(record->gen, false);
		}
		*/
	}
	else if (eventCode == characterTurnStart)
	{
		const auto record = reinterpret_cast<characterRecord*>(vTab[1]);
		if (record->gen
			&& record->faction->factionID == campaignHelpers::getCampaignData()->slaveFactionID
			&& record->gen->getTypeID() == characterTypeStrat::general
			&& record->gen->army
			&& record->gen->army->descrRebel
			&& record->gen->army->descrRebel->name
			&& (!record->labelCrypt || !eopCharacterDataDb::get()->getCharacterData(record->label))
			)
		{
			const auto rebelFac = eopRebelFactionDb::getRebelFaction(record->gen->army->descrRebel->name);
			if (rebelFac && !rebelFac->characterModels.empty())
			{
				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(rebelFac->characterModels.begin(), rebelFac->characterModels.end(), g);
				const auto randomModel = rebelFac->characterModels.front();
				stratModelsChange::setCharacterModel(record->gen, randomModel);
			}
		}
		
		/*
		if (record->gen
			&& record->faction->isPlayerControlled == 0
			&& record->gen->isGeneral()
			&& record->gen->army
			)
		{
			globalEopAiConfig::getInstance()->characterTurnStart(record->gen, true);
		}
		*/
		//globalEopAiConfig::getInstance()->characterTurnStart(record->gen, true);
	}
	else if (eventCode == factionTurnEnd)
	{
		const auto fac = reinterpret_cast<factionStruct*>(vTab[1]);
		if (globalEopAiConfig::getInstance()->getProductionHelper()->enabled)
		{
			globalEopAiConfig::getInstance()->getProductionHelper()->unitProducer(fac);
		}
		globalEopAiConfig::getInstance()->turnStartMove(fac, true);
		//if (globalEopAiConfig::getInstance()->enableLogging && !fac->isPlayerControlled)
		//	fac->aiFaction->aiGlobalStrategyDirector->militaryDirector.logData();
		//globalEopAiConfig::getInstance()->turnStartMove(fac, true);
		
		if (fac->isPlayerControlled == 0)
		{
			for (int i = fac->numOfCharacters - 1; i >= 0; i--)
			{
				if (const auto character = fac->characters[i]; 
					character && character->isGeneral() && character->army && !character->getSettlement())
				{
					globalEopAiConfig::getInstance()->characterTurnStart(character, true);
				}
			}
		}
		
		FIRST_END = true;
	}
	else if (eventCode == settlementTurnStart)
	{
		const auto sett = reinterpret_cast<settlementStruct*>(vTab[1]);
		if (const auto fac = sett->faction; fac->isPlayerControlled == 1 && sett->aiProductionController )
		{
			sett->aiProductionController->isAutoManagedTaxes = false;
		}
	}
	else if (eventCode == settlementTurnEnd)
	{
		const auto sett = reinterpret_cast<settlementStruct*>(vTab[1]);
		const auto region = stratMapHelpers::getRegion(sett->regionID);
		region->fixReligionLevels();
	}
	else if (eventCode == postBattle)
	{
		minHookFunctions::resetMercScrollCache();
		const auto campData = campaignHelpers::getCampaignData();
		campData->ignoreSpeedUp = false;
		if (campData->type == 3)
		{
			minorSettlementDb::clear();
		}
	}
	else if (eventCode == preBattleWithdrawal || eventCode == generalAssaultsResidence)
	{
		campaignHelpers::getCampaignData()->ignoreSpeedUp = false;
	}
	//else if (eventCode == settlementTurnStart)
	//{
	//	const auto sett = reinterpret_cast<settlementStruct*>(vTab[1]);
	//	if (sett->minorSettlementIndex > 3 || sett->minorSettlementIndex < 0)
	//	{
	//		const std::string str = "minorSettlementIndex failed to verify " + string(sett->name);
	//		gameHelpers::logStringGame(str);
	//		MessageBoxA(nullptr, "minorSettlementIndex failed to verify", "Attention", NULL);
	//	}
	//	if (static_cast<int8_t>(sett->isMinorSettlement) < 0 || static_cast<int8_t>(sett->isMinorSettlement) > 1)
	//	{
	//		const std::string str = "isMinorSettlement failed to verify " + string(sett->name);
	//		gameHelpers::logStringGame(str);
	//		MessageBoxA(nullptr, "isMinorSettlement failed to verify", "Attention", NULL);
	//	}
	//}
	else if (eventCode == buttonPressedCode && minHookFunctions::isEopCameraEnable)
	{
		char* str = reinterpret_cast<char*>(vTab[1]);
		if (strcmp(str, "prebattle_fight_button") == 0)
		{
		//	battleDataS* battle = battleHelpers::getBattleData();
			stratCamera.transitionToBattle();
		//	stratCamera.setPosition(battle->xCoord, battle->yCoord);
		}
	}
	
	gameEvents::onEventWrapper(eventCode, vTab);
}


void __fastcall patchesForGame::onLoadSaveFile(UNICODE_STRING**& savePath)
{
	LOADING_SAVE = true;
	const string relativePath = techFuncs::uniToAnsi(savePath);
	vector<string> files = techFuncs::getEopArchiveFiles(relativePath);
	if (files.empty())
	{
		onSaveGame(savePath);
		vector<string> newFiles = techFuncs::getEopArchiveFiles(relativePath);
		for (string& path : newFiles)
		{
			files.push_back(path);
		}
	}
	files = techFuncs::loadGameLoadArchive(files, savePath);
	minorSettlementDb::clear();
	settlementHelpers::clearOptionsDb();
	eopSettlementDataDb::get()->onGameLoad(files);
	eopFortDataDb::get()->onGameLoad(files);
	eopCharacterDataDb::get()->onGameLoad(files);
	eopFactionDataDb::get()->onGameLoad(files);
	gameStringDataDb::getInstance()->onGameLoad(files);
	gameEvents::onLoadGamePl(&files);
	plannedRetreatRoute::onGameLoad(files);
	techFuncs::deleteFiles(files);
	NEED_BUILD_FRONTIERS = true;
}

void cleanTempSaveFiles()
{
	std::string fPath = gameHelpers::getModPath();
	fPath += "\\eopData\\TempSaveData";
	filesystem::remove_all(fPath);
	filesystem::create_directory(fPath);
}

void __fastcall patchesForGame::onSaveGame(UNICODE_STRING**& savePath)
{
	vector<string>files;
	vector<string>* plugFiles = gameEvents::onSaveGamePl(savePath);
	for (string& path : *plugFiles)
	{
		files.push_back(path);
	}

	delete plugFiles;
	if (const std::string retreatsFile = plannedRetreatRoute::onGameSave(); !retreatsFile.empty())
		files.push_back(retreatsFile);
	cleanTempSaveFiles();
	if (const std::string settlementData = eopSettlementDataDb::get()->onGameSave(); !settlementData.empty())
		files.push_back(settlementData);
	if (const std::string fortData = eopFortDataDb::get()->onGameSave(); !fortData.empty())
		files.push_back(fortData);
	if (const std::string characterData = eopCharacterDataDb::get()->onGameSave(); !characterData.empty())
		files.push_back(characterData);
	if (const std::string factionData = eopFactionDataDb::get()->onGameSave(); !factionData.empty())
		files.push_back(factionData);
	if (const std::string gameStringData = gameStringDataDb::getInstance()->onGameSave(); !gameStringData.empty())
		files.push_back(gameStringData);
	techFuncs::saveGameMakeArchive(savePath, files);
	techFuncs::deleteFiles(files);
}

void __fastcall patchesForGame::onTileCheck(int* coords)
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "onTileCheck" << endl;
	f1.close();
#endif
	if (coords == nullptr)
	{
		MessageBoxA(NULL, "Error in onTileCheck", "Attention", NULL);
		return;
	}

	tilesChange::onTilesCheck(coords);
}

void __stdcall patchesForGame::readTilesFile()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "readTilesFile" << endl;
	f1.close();
#endif
}

void __stdcall patchesForGame::unicalStratModsRead()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "unicalStratModsRead" << endl;
	f1.close();
#endif
	gameHelpers::logStringGame("Loading strat models...");
	stratModelsChange::loadModels();
	gameHelpers::logStringGame("Strat models loaded");
}

void __stdcall patchesForGame::loadCharStratModels()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "loadCharStratModels" << endl;
	f1.close();
#endif
	gameHelpers::logStringGame("Loading strat characters models...");
	stratModelsChange::loadCharModels();
	gameHelpers::logStringGame("Strat character models loaded");
}


void __stdcall patchesForGame::checkAndChangeModels()
{
#if defined TESTPATCHES
	ofstream f1("logs\\TESTPATCHES.log", ios::app);

	f1 << "checkAndChangeModels" << endl;
	f1.close();
#endif
	stratModelsChange::checkAndChangeStratModels();
}

struct surfaceEntry
{
	gameStdVector<int> walls;
	gameStdVector<int> defenses;
	int id;
	float health;
};

struct weakPointEntry
{
	buildingBattle* building;
	int surface;
	gameStdVector<int> defenses;
	float health;
};

struct defenseEntry
{
	buildingBattle* building;
	gameStdVector<int> surfaces;
	gameStdVector<int> gates;
};

struct positionEntry
{
	float posX;
	float posy;
	float dirX;
	float dirY;
	gameStdVector<int> dependants;
	int priority;
	int flags;
};

struct streetEntry
{
	float posX;
	float posy;
	float dirX;
	float dirY;
	positionEntry* position;
};

struct perimeterEntry
{
	gameStdVector<surfaceEntry> surfaces;
	gameStdVector<weakPointEntry> weakPoints;
	gameStdVector<defenseEntry> defenses;
	gameStdVector<streetEntry> streets;
	int breaches;
	int defAvailable;
	int manned;
	int neutralized;
};

struct analyzerWeakPoint
{
	float threat;
	weakPointEntry* entry;
};

struct analyzerSurface
{
	float threat;
	surfaceEntry* entry;
};

struct analyzerStreet
{
	streetEntry* entry;
	float threat;
};

struct analyzerArty
{
	float posX;
	float posY;
	uint8_t flags;
	uint8_t pad[3];
};

struct defenseThreat
{
	unit* unit;
	siegeEngine* engine;
};

struct defenseAnalyzerPerimeter
{
	gameStdVector<defenseThreat> threats;
	float enemyMeleeStr;
	float enemyMissileStr;
	float enemyMeleeProportion;
	float enemyStr;
	float allyMeleeStr;
	float allyMissileStr;
	float allyMeleeProportion;
	float allyStr;
	float friendlyMeleeStr;
	float friendlyMissileStr;
	float friendlyMeleeProportion;
	float friendlyStr;
	gameStdVector<analyzerWeakPoint> breaches;
	gameStdVector<analyzerWeakPoint> assaults;
	gameStdVector<analyzerSurface> surfaces;
	gameStdVector<analyzerStreet> streets;
	gameStdVector<analyzerArty> artillery;
};

struct settlementAnalyzer
{
	gameStdVector<defenseAnalyzerPerimeter> perimeters;
	float allyStr;
	float allyMeleeStr;
	float enemyStr;
	float enemyMeleeStr;
	float friendlyStr;
	float friendlyMeleeStr;
	float plazaThreat;
	bool  inSettlement;
	int  ticks;
	battleResidence* residence;
	battleAI* battleAi;
};


void patchesForGame::threatDebugWalls(void *perimeter)
{
	const auto analyzer = static_cast<defenseAnalyzerPerimeter*>(perimeter);
	
	for (int i = 0; i < analyzer->breaches.size(); ++i)
	{
		const auto breach = analyzer->breaches[i];
		
		if(breach.threat < 1e-3f || !breach.entry)
			break;
	}
}

int __fastcall patchesForGame::onCallAuto(void* resolveData)
{
	if (const auto campaign = campaignHelpers::getCampaignData(); campaign->turnNumber == 0 || !campaign->setupAiBattle())
	{
		
		GAME_FUNC(void(__thiscall*)(void*, bool), autoResolveHandler)(resolveData, true);
		return 0;
	}
	return 1;
}

void __stdcall patchesForGame::battleLoaded()
{
	discordManager::battleLoaded();
	battleCreator::startCreationThread();
}

void __stdcall patchesForGame::toReadGameDbsAtStart()
{
	//	plugins::onReadGameDbsAtStart();
}

void __stdcall patchesForGame::onDrawPartsOfStratObjects()
{
	graphicsD3D::onDrawPartsOfStratObjects();
}

void __stdcall patchesForGame::onDrawAll()
{
	graphicsD3D::onDrawAllGameStuff();
}

void __stdcall patchesForGame::onStartOfDrawFunction()
{
}

void __stdcall patchesForGame::onRetreat()
{
	plannedRetreatRoute::onRetreat();
}

void __fastcall patchesForGame::onStopCharacter(character* character)
{
	if (const auto campaign = campaignHelpers::getCampaignData(); campaign->humanPlayers < 2)
		character->isStopCharacterNeeded = 1;
}

eduEntry* __fastcall patchesForGame::recruitEopUnit(int eduIndex)
{
	return eopDu::getEduEntry(eduIndex);
}

void __fastcall patchesForGame::recruitEopUnit2(int eduIndex)
{
	if (eduEntry* entry = eopDu::getEduEntry(eduIndex))
	{
		entry->unitCreatedCounter++;
	}
}

eduEntry* __fastcall patchesForGame::onReadDescrRebel(DWORD value)
{
	const DWORD eduIndex = (value * 4) / 996;
	return eopDu::getEduEntry(eduIndex);
}

void __fastcall patchesForGame::recruitEopMercUnit(DWORD pad, DWORD pad2, regionStruct* region, int eduIndex, int factionID, int exp)
{
	int regionID = region->regionID;
	unitHelpers::createUnitIdx(eduIndex, regionID, factionID, exp, 0, 0);
}

void WINAPI patchesForGame::onMoveRecruitQueue()
{
	// if (const auto campaign = campaignHelpers::getCampaignData(); campaign->humanPlayers < 2)
	// 	return;
	// MessageBoxA(nullptr, "Moving queue disabled in hotseat mode. EXIT NOW!!!!!", "ATTENTION!", NULL);
	// std::terminate();
}

void __fastcall patchesForGame::onEndSiege(settlementStruct* sett)
{
	const int x = sett->xCoord;
	const int y = sett->yCoord;
	gameEvents::onEndSiege(x, y);
}

void __fastcall patchesForGame::onStartSiege(settlementStruct* sett)
{
	if (const auto campaign = campaignHelpers::getCampaignData(); campaign and campaign->isOpen)
		campaign->ignoreSpeedUp = false;
	if (sett->siegeNum == 0)
	{
		return;
	}
	const int x = sett->xCoord;
	const int y = sett->yCoord;
	gameEvents::onStartSiege(x, y);
}

void __fastcall patchesForGame::onLoadDescrBattleCharacter(armyStruct* army, character* goalGen)
{
	characterHelpers::setBodyguard(goalGen, army->units[0]);//we replace game function what set army leader character.

	
	std::string relativePath = techFuncs::uniToAnsi(campaignHelpers::getCampaignData()->currentDescrFile);

	if (relativePath.find("battle") != std::string::npos)
	{
		battleCreator::addCharactersToCustomBattleArmy(army, relativePath);
	}
}

void __stdcall patchesForGame::onBattleStateChange()
{
	//results screen
	if (battleHelpers::getBattleData()->battleState == 9)
	{
		battleCreator::onBattleResultsScreen();
	}
}

void __thiscall patchesForGame::onFactionSymbolSelect()
{
	int factionID = *reinterpret_cast<int*>(dataOffsets::offsets.selectedFactionSymbol);
	gameEvents::onFactionSymbolSelect(factionID);
}

void __thiscall patchesForGame::onCharacterDied(character* character)
{
	gameEvents::onCharacterDied(character);
}

void __thiscall patchesForGame::onFactionDied(factionStruct* faction)
{
	gameEvents::onFactionDied(faction);
}

struct
{
	bool isComingFromConsole = false;
}logonConsoleD;
void __stdcall consolePatches::onGameConsoleCommandFromConsole()
{
	logonConsoleD.isComingFromConsole = true;
}

void __stdcall consolePatches::onGameConsoleCommandFromScript()
{
	logonConsoleD.isComingFromConsole = false;
}

int __fastcall consolePatches::onReadLogonOrLogoff(int isLogonNow)
{
	if (logonConsoleD.isComingFromConsole == true)
	{
		return isLogonNow;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// MIN HOOK FUNCTIONS //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

minHookFunctions::t_onUnitCreate                           minHookFunctions::o_onUnitCreate = nullptr;
minHookFunctions::t_onMaybeWillSpyOpenGates                minHookFunctions::o_onMaybeWillSpyOpenGates = nullptr;
minHookFunctions::t_onCharacterSwitchFaction               minHookFunctions::o_onCharacterSwitchFaction = nullptr;
minHookFunctions::t_onCreateWife                           minHookFunctions::o_onCreateWife = nullptr;
minHookFunctions::t_onCreateMessageAboutMarriage           minHookFunctions::o_onCreateMessageAboutMarriage = nullptr;
minHookFunctions::t_onCreateCandidateMarrying              minHookFunctions::o_onCreateCandidateMarrying = nullptr;
minHookFunctions::t_onDaughterReadyMarryHusband            minHookFunctions::o_onDaughterReadyMarryHusband = nullptr;
minHookFunctions::t_onCalculationRatioForBirth             minHookFunctions::o_onCalculationRatioForBirth = nullptr;
minHookFunctions::t_debugRenderLine                        minHookFunctions::o_debugRenderLine = nullptr;
minHookFunctions::t_debugLineAdd                           minHookFunctions::o_debugLineAdd = nullptr;
minHookFunctions::t_debugRenderPeg                         minHookFunctions::o_debugRenderPeg = nullptr;
minHookFunctions::t_debugRenderCircle                      minHookFunctions::o_debugRenderCircle = nullptr;
minHookFunctions::t_displayMissileStats                    minHookFunctions::o_displayMissileStats = nullptr;
minHookFunctions::t_displayMeleeStats                      minHookFunctions::o_displayMeleeStats = nullptr;
minHookFunctions::t_displayArmourStats                     minHookFunctions::o_displayArmourStats = nullptr;
minHookFunctions::t_displayDefenseStats                    minHookFunctions::o_displayDefenseStats = nullptr;
minHookFunctions::t_fleeConstructor                        minHookFunctions::o_fleeConstructor1 = nullptr;
minHookFunctions::t_fleeConstructor                        minHookFunctions::o_fleeConstructor2 = nullptr;
minHookFunctions::t_onBlockadePort                         minHookFunctions::o_onBlockadePort = nullptr;
minHookFunctions::t_onQuickLoading                         minHookFunctions::o_onQuickLoading = nullptr;
minHookFunctions::t_onUnitDestroy                          minHookFunctions::o_onUnitDestroy = nullptr;
minHookFunctions::t_onCreateTooltip                        minHookFunctions::o_onCreateTooltip = nullptr;
minHookFunctions::t_onCreateTooltipByCoords                minHookFunctions::o_onCreateTooltipByCoords = nullptr;
minHookFunctions::t_onCreateCapabilityUnicodeString        minHookFunctions::o_onCreateCapabilityUnicodeString = nullptr;
minHookFunctions::t_onPlayGameSound                        minHookFunctions::o_onPlayGameSound = nullptr;
minHookFunctions::t_onPlayCharacterVoice                   minHookFunctions::o_onPlayCharacterVoice = nullptr;
minHookFunctions::t_onHudElementClicked                    minHookFunctions::o_onHudElementClicked = nullptr;
minHookFunctions::t_onSetElementPosition                   minHookFunctions::o_onSetElementPosition = nullptr;
minHookFunctions::t_onCreateAdvancedSettlementInfoScroll   minHookFunctions::o_onCreateAdvancedSettlementInfoScroll = nullptr;
minHookFunctions::t_onResetUnitRecruitAndRetrainScroll     minHookFunctions::o_onResetUnitRecruitAndRetrainScroll = nullptr;
minHookFunctions::t_onTraitsListConstructor1               minHookFunctions::o_onTraitsListConstructor1 = nullptr;
minHookFunctions::t_onTraitsListConstructor2               minHookFunctions::o_onTraitsListConstructor2 = nullptr;
minHookFunctions::t_onReinitNamedCharacterInfoScroll       minHookFunctions::o_onReinitNamedCharacterInfoScroll = nullptr;
minHookFunctions::t_onResetTraitsListHeight                minHookFunctions::o_onResetTraitsListHeight = nullptr;
minHookFunctions::t_onMercenaryScrollInit                  minHookFunctions::o_onMercenaryScrollInit = nullptr;
minHookFunctions::t_onSetSliderState                       minHookFunctions::o_onSetSliderState = nullptr;
minHookFunctions::t_onElementReset                         minHookFunctions::o_onElementReset = nullptr;
minHookFunctions::t_onSetColor                             minHookFunctions::o_onSetColor = nullptr;
minHookFunctions::t_onCreateDebugInfoText                  minHookFunctions::o_onCreateDebugInfoText = nullptr;
minHookFunctions::t_onCharacterInfoFrameInit               minHookFunctions::o_onCharacterInfoFrameInit = nullptr;
minHookFunctions::t_onCameraMove                           minHookFunctions::o_onCameraMove = nullptr;
minHookFunctions::t_onCameraAction                         minHookFunctions::o_onCameraAction = nullptr;
minHookFunctions::t_onAerialMapInit                        minHookFunctions::o_onAerialMapInit = nullptr;
minHookFunctions::t_onAerialMapUpdate                      minHookFunctions::o_onAerialMapUpdate = nullptr;
minHookFunctions::t_onAerialMapSetPosition                 minHookFunctions::o_onAerialMapSetPosition = nullptr;
minHookFunctions::t_onMoveStratCameraSlow                  minHookFunctions::o_onMoveStratCameraSlow = nullptr;
minHookFunctions::t_onZoomStratCamera                      minHookFunctions::o_onZoomStratCamera = nullptr;
minHookFunctions::t_onCalcPathFindWidth                    minHookFunctions::o_onCalcPathFindWidth = nullptr;

DWORD minHookFunctions::lastSoundClass = NULL;
bool minHookFunctions::isUnlockWeaponLimit = false;
oneTile* minHookFunctions::selectTile = nullptr;
bool minHookFunctions::openConsole = false;
coordPair minHookFunctions::rememberCoords{ 0, 0 };
DWORD minHookFunctions::lastTooltipClass1 = NULL;
void* minHookFunctions::lastTooltipClass2 = NULL;
UNICODE_STRING*** minHookFunctions::lastTooltipUniString = NULL;
int minHookFunctions::arrowClick = 0;
int minHookFunctions::arrowRememberY = 53;
bool minHookFunctions::arrowChange = false;
int minHookFunctions::hudBuildings = NULL;
advancedSettlementInfoScroll* minHookFunctions::advancedSettlementInfoScrollClass = NULL;
DWORD minHookFunctions::unitRecruitAndRetrainScrollClass = NULL;
bool minHookFunctions::isBlockSlider = false;
int minHookFunctions::traitsChildWindowHeight = 92;
int minHookFunctions::mercsMaxSlots = 18;
int minHookFunctions::mercsNumOfLine = 3;
bool minHookFunctions::isReset = false;
mercenaryScroll* minHookFunctions::lastMercScroll = NULL;
sliderStruct* minHookFunctions::lastSliderStruct = NULL;
bool minHookFunctions::isMercenaryScrollOpen = false;
map<int, int> minHookFunctions::bufferPosY;
unordered_map<uint32_t, rgba> minHookFunctions::rgbaColors;
bool minHookFunctions::isDebugInfoOpen = false;
string minHookFunctions::debugInfoText = "debug";
direct3DTextureStruct* minHookFunctions::rememberTextures = NULL;
bool minHookFunctions::isEopCameraEnable = false;


MH_STATUS minHookFunctions::hook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal, string function)
{
	MH_CreateHook(pTarget, pDetour, ppOriginal);
	MH_STATUS result = MH_EnableHook((char*)pTarget);

	gameHelpers::logStringGame("MIN_HOOK(" + function + " --> " + pointerToString(pTarget) + ")");

	return result;
}

void minHookFunctions::init()
{
	MIN_HOOK(a_onUnitCreate,                           onUnitCreate,                           o_onUnitCreate);
	MIN_HOOK(a_onMaybeWillSpyOpenGates,                onMaybeWillSpyOpenGates,                o_onMaybeWillSpyOpenGates);
	MIN_HOOK(a_onCharacterSwitchFaction,               onCharacterSwitchFaction,               o_onCharacterSwitchFaction);
	MIN_HOOK(a_onCreateWife,                           onCreateWife,                           o_onCreateWife);
	MIN_HOOK(a_onCreateMessageAboutMarriage,           onCreateMessageAboutMarriage,           o_onCreateMessageAboutMarriage);
	MIN_HOOK(a_onCreateCandidateMarrying,              onCreateCandidateMarrying,              o_onCreateCandidateMarrying);
	MIN_HOOK(a_onDaughterReadyMarryHusband,            onDaughterReadyMarryHusband,            o_onDaughterReadyMarryHusband);
	MIN_HOOK(a_debugRenderLine,                        debugRenderLine,                        o_debugRenderLine);
	MIN_HOOK(a_debugLineAdd,                           debugLineAdd,                           o_debugLineAdd);
	MIN_HOOK(a_debugRenderPeg,                         debugRenderPeg,                         o_debugRenderPeg);
	MIN_HOOK(a_debugRenderCircle,                      debugRenderCircle,                      o_debugRenderCircle);
	MIN_HOOK(a_displayMissileStats,                    displayMissileStats,                    o_displayMissileStats);
	MIN_HOOK(a_displayMeleeStats,                      displayMeleeStats,                      o_displayMeleeStats);
	MIN_HOOK(a_displayArmourStats,                     displayArmourStats,                     o_displayArmourStats);
	MIN_HOOK(a_displayDefenseStats,                    displayDefenseStats,                    o_displayDefenseStats);
	MIN_HOOK(a_fleeConstructor1,                       fleeConstructor1,                       o_fleeConstructor1);
	MIN_HOOK(a_fleeConstructor2,                       fleeConstructor2,                       o_fleeConstructor2);
	MIN_HOOK(a_onCalculationRatioForBirth,             onCalculationRatioForBirth,             o_onCalculationRatioForBirth);
	MIN_HOOK(a_onBlockadePort,                         onBlockadePort,                         o_onBlockadePort);
	MIN_HOOK(a_onQuickLoading,                         onQuickLoading,                         o_onQuickLoading);
	MIN_HOOK(a_onUnitDestroy,                          onUnitDestroy,                          o_onUnitDestroy);
	MIN_HOOK(a_onCreateTooltip,                        onCreateTooltip,                        o_onCreateTooltip);
	MIN_HOOK(a_onCreateTooltipByCoords,                onCreateTooltipByCoords,                o_onCreateTooltipByCoords);
	MIN_HOOK(a_onCreateCapabilityUnicodeString,        onCreateCapabilityUnicodeString,        o_onCreateCapabilityUnicodeString);
	MIN_HOOK(playGameSoundAdd,                         onPlayGameSound,                        o_onPlayGameSound);
//	MIN_HOOK(playCharacterVoiceAdd,                    onPlayCharacterVoice,                   o_onPlayCharacterVoice);
	MIN_HOOK(a_onHudElementClicked,                    onHudElementClicked,                    o_onHudElementClicked);
	MIN_HOOK(a_onSetElementPosition,                   onSetElementPosition,                   o_onSetElementPosition);
	MIN_HOOK(a_onCreateAdvancedSettlementInfoScroll,   onCreateAdvancedSettlementInfoScroll,   o_onCreateAdvancedSettlementInfoScroll);
	MIN_HOOK(a_onResetUnitRecruitAndRetrainScroll,     onResetUnitRecruitAndRetrainScroll,     o_onResetUnitRecruitAndRetrainScroll);
	MIN_HOOK(a_onTraitsListConstructor1,               onTraitsListConstructor1,               o_onTraitsListConstructor1);
	MIN_HOOK(a_onTraitsListConstructor2,               onTraitsListConstructor2,               o_onTraitsListConstructor2);
	MIN_HOOK(a_onReinitNamedCharacterInfoScroll,       onReinitNamedCharacterInfoScroll,       o_onReinitNamedCharacterInfoScroll);
	MIN_HOOK(a_onResetTraitsListHeight,                onResetTraitsListHeight,                o_onResetTraitsListHeight);
	MIN_HOOK(a_onMercenaryScrollInit,                  onMercenaryScrollInit,                  o_onMercenaryScrollInit);
	MIN_HOOK(a_onSetSliderState,                       onSetSliderState,                       o_onSetSliderState);
	MIN_HOOK(a_onElementReset,                         onElementReset,                         o_onElementReset);
	MIN_HOOK(a_onSetColor,                             onSetColor,                             o_onSetColor);
	MIN_HOOK(a_onCreateDebugInfoText,                  onCreateDebugInfoText,                  o_onCreateDebugInfoText);
//	MIN_HOOK(a_onCharacterInfoFrameInit,               onCharacterInfoFrameInit,               o_onCharacterInfoFrameInit);
	MIN_HOOK(a_onCameraMove,                           onCameraMove,                           o_onCameraMove);
	MIN_HOOK(a_onCameraAction,                         onCameraAction,                         o_onCameraAction);
	MIN_HOOK(a_onAerialMapInit,                        onAerialMapInit,                        o_onAerialMapInit);
	MIN_HOOK(a_onAerialMapUpdate,                      onAerialMapUpdate,                      o_onAerialMapUpdate);
	MIN_HOOK(a_onAerialMapSetPosition,                 onAerialMapSetPosition,                 o_onAerialMapSetPosition);
	MIN_HOOK(moveStratCameraSlowFunc,                  onMoveStratCameraSlow,                  o_onMoveStratCameraSlow);
	MIN_HOOK(zoomStratCameraFunc,                      onZoomStratCamera,                      o_onZoomStratCamera);
	MIN_HOOK(onCalcPathFindWidth,                      onCalcPathFindWidth,                    o_onCalcPathFindWidth);
}

/* prevents infinite loop in pathfinding code, game bug with invalid float argument */
bool __thiscall minHookFunctions::onCalcPathFindWidth(void* _this, vector2* start, vector2* end, float maxWidth, float* outWidth, int swim, int maxSlope, int validZone)
{
	if (!(maxWidth >= 0.f && maxWidth <= 2048.f))                                                                                                                                                                                    
	{                                                                                                                                                                                                                               
		gameHelpers::logStringGame("onCalcPathFindWidth: maxWidth out of range or NaN, resetting to 2048.f");                                                                                                                           
		maxWidth = 2048.f;                                                                                                                                                                                                       
	}  
	
	return o_onCalcPathFindWidth(_this, start, end, maxWidth, outWidth, swim, maxSlope, validZone);
}

int __thiscall minHookFunctions::debugLineAdd(void* _this, vector3* start, vector3* end, color8888 color, float time, bool zbuffered)
{
	return battleHelpers::addDebugLine(start, end, color.asInt(), time);
}

void __cdecl minHookFunctions::debugRenderLine(vector2* start, vector2* end, color8888 color, float time, bool zbuffered, float hOffset)
{
	vector2 dir{};
	dir.x = end->x - start->x;
	dir.y = end->y - start->y;

	float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
	const auto inv = 1.f / length;
	dir.x *= inv;
	dir.y *= inv;

	color.r = 0;
	color.b = 0;

	constexpr float pointSeparation = 10.f;

	vector3 startLand(0.f, 0.f, 0.f);
	vector3 endLand(start->x, battleHelpers::getBattleMapHeight(start->x, start->y) + 1.f + hOffset, start->y);

	while (length > 0.f)
	{
		startLand = endLand;
		float separation = 0.0f;

		if (length >= pointSeparation)
		{
			separation = pointSeparation;
		}
		else
		{
			separation = length;
		}
		length -= separation;

		vector2 end2d (endLand.x, endLand.y);
		end2d.x += dir.x * separation;
		end2d.y += dir.y * separation;
		endLand = vector3(end2d.x, battleHelpers::getBattleMapHeight(end2d.x, end2d.y) + 1.f + hOffset, end2d.y);

		battleHelpers::addDebugLine(&startLand, &endLand, color.asInt(), time);
	}
}

void minHookFunctions::debugRenderPeg(vector2* start, float height, color8888 color, float time)
{
	vector3 bottom(start->x, battleHelpers::getBattleMapHeight(start->x, start->y) + 0.1f, start->y);
	vector3 top(start->x, height, start->y);

	color.r = 0;
	color.b = 0;
	
	battleHelpers::addDebugLine(&bottom, &top, color.asInt(), time);
}

int minHookFunctions::displayMissileStats(eduEntry* entry, int exp, int wpn)
{
	if (entry->engineStats.isValid)
		return entry->engineStats.attack;
	if (m2tweopOptions::enableRangedWeaponUpg)
		return entry->priStats.attack + wpn * m2tweopOptions::weaponBonusModifier;
	return entry->priStats.attack;
}

int combatExperienceBonuses[10] = {0, 1, 1, 1, 2, 2, 2, 3, 3, 3};

int minHookFunctions::displayMeleeStats(eduEntry* entry, int exp, int wpn)
{
	int att = entry->priStats.attack;
	exp = max(exp, 0);
	const auto xpBonus = combatExperienceBonuses[min(exp, 9)];
	if (entry->animalStats.isValid)
		att = entry->animalStats.attack;
	else if (entry->mountStats.isValid)
		att = entry->mountStats.attack;
	else if (entry->priStats.isMissile || entry->priStats.isPrec)
		att = entry->secStats.attack;
	
	return att + xpBonus + wpn * m2tweopOptions::weaponBonusModifier;
}

int minHookFunctions::displayArmourStats(eduEntry* entry, int armourUpg)
{
	int armour = entry->statPriArmour.armour;
	if (entry->statArmourAnimal.isValid)
		armour = entry->statArmourAnimal.armour;
	else if (entry->statArmourMount.isValid)
		armour = entry->statArmourMount.armour;
	
	return armour + armourUpg * m2tweopOptions::armourBonusModifier;
}

int minHookFunctions::displayDefenseStats(eduEntry* entry, int exp, int armourUpg)
{
	int def = entry->statPriArmour.defense;
	int shield = 0;
	int armour = entry->statPriArmour.armour;
	if (entry->statArmourAnimal.isValid)
	{
		def = entry->statArmourAnimal.defense;
		armour = entry->statArmourAnimal.armour;
	}
	else if (entry->statArmourMount.isValid)
	{
		def = entry->statArmourMount.defense;
		armour = entry->statArmourMount.armour;
	}
	else
	{
		shield = entry->statPriArmour.shield;
	}

	return def + shield + armour + armourUpg * m2tweopOptions::armourBonusModifier;
}

void* minHookFunctions::fleeConstructor1(void* flee, void* sett, void* ch, bool action, bool back, bool clearMp)
{
	if (m2tweopOptions::getNoMovementResetOnFlee())
		return o_fleeConstructor1(flee, sett, ch, action, back, false);
	
	return o_fleeConstructor1(flee, sett, ch, action, back, clearMp);
}

void* minHookFunctions::fleeConstructor2(void* flee, void* sett, void* ch, bool action, bool back, bool clearMp)
{
	if (m2tweopOptions::getNoMovementResetOnFlee())
		return o_fleeConstructor2(flee, sett, ch, action, back, false);
	
	return o_fleeConstructor2(flee, sett, ch, action, back, clearMp);
}

constexpr float pi = 3.1415926535897932333797165867879296635503123989707390137482903185973555f;

int16_t rad2tab2(float rad)
{
	return static_cast<int16_t>(roundf(rad * *reinterpret_cast<float*>(dataOffsets::offsets.rad2tab)));
}

void minHookFunctions::debugRenderCircle(vector2* center, float radius, int segments, color8888 color, float time)
{
	const float segments_recip = 1.f / static_cast<float>(segments);

	for (auto i = 0;  i < segments; ++i)
	{
		const int16_t startAngle = rad2tab2(2.f * pi * static_cast<float>(i) * segments_recip);
		const int16_t endAngle = rad2tab2(2.f * pi * static_cast<float>(i + 1) * segments_recip);

		vector2 start;
		const auto sineValues = battleHelpers::getSineValues();
		start.x = center->x + sineValues[static_cast<uint16_t>(startAngle) >> 2] * radius;
		start.y = center->y + sineValues[static_cast<uint16_t>(startAngle + 0x4000) >> 2] * radius;

		vector2 end;
		end.x = center->x + sineValues[static_cast<uint16_t>(endAngle) >> 2] * radius;
		end.y = center->y + sineValues[static_cast<uint16_t>(endAngle + 0x4000) >> 2] * radius;

		debugRenderLine(&start, &end, color, time, false, 0.f);
	}
}

// Called when uploading files descr_strat.txt and descr_battle.txt, as well as when creating rebel units.   
unit* __thiscall minHookFunctions::onUnitCreate(unitDb* _this, regionStruct* region, stringWithHash* id, int factionID, int combat_ability, int soldiers, int armour_lvl, int weapon_lvl)
{
	if (!isUnlockWeaponLimit)
	{
		return o_onUnitCreate(_this, region, id, factionID, combat_ability, soldiers, armour_lvl, weapon_lvl);
	}

	if (weapon_lvl < 0)
	{
		weapon_lvl = 0;
	}
	else if (weapon_lvl > 3)
	{
		weapon_lvl = 3;
	}

	MemWork::WriteData(&weapon_lvl, dataOffsets::offsets.weaponLimit10, 4);

	return o_onUnitCreate(_this, region, id, factionID, combat_ability, soldiers, armour_lvl, weapon_lvl);
}

bool __thiscall minHookFunctions::onMaybeWillSpyOpenGates(void* _this, character* general)
{
	bool result = o_onMaybeWillSpyOpenGates(_this, general);

	if (m2tweopOptions::isOpenGateOnStratMapActive != 0)
	{
		result = m2tweopOptions::isOpenGateOnStratMap;
		m2tweopOptions::isOpenGateOnStratMapActive--;
		m2tweopOptions::isOpenGateOnStratMapActive = max(m2tweopOptions::isOpenGateOnStratMapActive, 0);
	}

	return result;
}

void __thiscall minHookFunctions::onCharacterSwitchFaction(character* _this, factionStruct* faction, int param_2, int param_3)
{
	gameEvents::onCharacterSwitchFaction(_this, faction);

	o_onCharacterSwitchFaction(_this, faction, param_2, param_3);
}

//### Add a Lua event function, for example to override a spouse. 
characterRecord* __thiscall minHookFunctions::onCreateWife(characterRecord* husband)
{
	return o_onCreateWife(husband);
}

void __cdecl minHookFunctions::onCreateMessageAboutMarriage(characterRecord* husband, characterRecord* new_wife, marriageOption* mo)
{
	o_onCreateMessageAboutMarriage(husband, new_wife, mo);
}

//### Add a Lua event function, for example to override a spouse. 
characterRecord* __thiscall minHookFunctions::onCreateCandidateMarrying(characterRecord* daughter)
{
	return o_onCreateCandidateMarrying(daughter);
}

void __cdecl minHookFunctions::onDaughterReadyMarryHusband(characterRecord* daughter, characterRecord* new_husband, marriageOption* mo)
{
	o_onDaughterReadyMarryHusband(daughter, new_husband, mo);
}

float __fastcall minHookFunctions::onCalculationRatioForBirth(family* _this)
{
	int buffer = _this->faction->regionsNum;
	_this->faction->regionsNum = gameEvents::onCalculationRatioForBirth(_this->faction, _this->faction->regionsNum);

	float result = o_onCalculationRatioForBirth(_this);
	_this->faction->regionsNum = buffer;

	return result;
}

void __thiscall minHookFunctions::onBlockadePort(character* _this, portBuildingStruct* port)
{
	o_onBlockadePort(_this, port);
	gameEvents::onBlockadePort(_this, port);
	if (port->fac->isPlayerControlled == 1)
		createBlockadePortMessage(_this->army, port->settlement);
}

static std::optional<filesystem::path> findNewestFileWithExtension(const string& folder_path, const string& extension)
{
	std::optional<filesystem::path> newest_file;
	if (extension.empty() || extension[0] != '.') 
	{
		return newest_file;
	}
	filesystem::file_time_type newest_time{};
	bool first = true;
	for (const auto& entry : filesystem::directory_iterator(folder_path))
	{
		if (!entry.is_regular_file()) 
		{
			continue;
		}
		const auto& path = entry.path();
		if (path.extension() != extension) 
		{
			continue;
		}
		auto time = filesystem::last_write_time(path);
		if (first || time > newest_time) 
		{
			newest_time = time;
			newest_file = path;
			first = false;
		}
	}
	return newest_file;
}

void __cdecl minHookFunctions::onQuickLoading(UNICODE_STRING**& savePath)
{
	string save = techFuncs::uniToAnsi(savePath);
	filesystem::path filePath = save;
	string ext = filePath.extension().string();
	if (ext != ".sav")
	{
		const std::string folder = globals::dataS.modPath + "\\saves";
		auto result = findNewestFileWithExtension(folder, ".sav");
		if (result)
		{
			auto newSave = result->string();
			log("minHookFunctions::onQuickLoading: fix save: old: " + save + " --> new: " + newSave);
			gameStringHelpers::createUniString(savePath, newSave.c_str());
		}
		else
		{
			log("[error] minHookFunctions::onQuickLoading: failed to load save file: " + save);
			return;
		}
	}
	o_onQuickLoading(savePath);
}

void __thiscall minHookFunctions::onUnitDestroy(armyStruct* _this, unit* unit)
{
	gameEvents::onUnitDestroy(_this, unit);
	o_onUnitDestroy(_this, unit);
}

/////////////////////////
///=== UI ELEMENTS ===///
/////////////////////////

void __thiscall minHookFunctions::onCreateTooltip(DWORD _this, void* p, UNICODE_STRING*** u)
{
	if (u)
	{
		lastTooltipClass1    = _this;
		lastTooltipClass2    = p;
		lastTooltipUniString = u;

		UNICODE_STRING** uni = *u;
		string str = gameStringHelpers::uniStringToStr(uni);
		if (str.size() > 0)
		{
			string* newStrP = gameEvents::onCreateTooltip(str.c_str());
			string newStr = *newStrP;
			delete newStrP;
			if (newStr == "disable")
			{
				u = nullptr;
			}
			else if (newStr.size() > 0 && newStr != str)
			{
				gameStringHelpers::createUniString(uni, newStr.c_str());
				u = &uni;
			}
		}
	}
	o_onCreateTooltip(_this, p, u);
}

void __thiscall minHookFunctions::onCreateTooltipByCoords(void* _this, coordPair* coords, UNICODE_STRING**& u1, UNICODE_STRING**& u2)
{
	if (oneTile* tile = stratMapHelpers::getTile(coords->xCoord, coords->yCoord); tile && !tile->object)
	{
		string* newStrP = gameEvents::onCreateTooltipByCoords(coords->xCoord, coords->yCoord);
		string newStr = *newStrP;
		delete newStrP;
		if (newStr.size() > 0)
		{
			gameStringHelpers::createUniString(u1, newStr.c_str());
		}
	}
	o_onCreateTooltipByCoords(_this, coords, u1, u2);
}

UNICODE_STRING**& __thiscall minHookFunctions::onCreateCapabilityUnicodeString(buildingLevelCapability* _this, void* param_1, settlementStruct* settlement)
{
	if (_this->bonus < 0)
	{
		BYTE plus = 0x20;
		const DWORD add = dataOffsets::offsets.plusMinusFix + 0x3;
		MemWork::WriteData(&plus, add, 1);
	}

	UNICODE_STRING**& result = o_onCreateCapabilityUnicodeString(_this, param_1, settlement);

	if (_this->bonus < 0)
	{
		BYTE plus = 0x2B;
		const DWORD add = dataOffsets::offsets.plusMinusFix + 0x3;
		MemWork::WriteData(&plus, add, 1);
	}

	return result;
}

//### Add a Lua event function, for example to override sound events? 
void __cdecl minHookFunctions::onPlayGameSound(DWORD _this, int sound)
{
	if (!_this) return;

	lastSoundClass = _this;

//	log("onPlayGameSound: " + to_string(sound));

	//addUnitToQueue || removeUnit   || draggingUnit   || buildings
	if (sound == 151 || sound == 152 || sound == 132 /*|| sound == 145 || sound == 146*/)
	{
		isBlockSlider = true;
	}
	/*//settlementSelected
	else if (sound == 178)
	{
		isBlockSlider = false;
		if (unitRecruitAndRetrainScrollClass)
		{
			o_onResetUnitRecruitAndRetrainScroll(unitRecruitAndRetrainScrollClass);
		}
		if (hudBuildings)
		{
			coordPair position{ 273, 53 };
			o_onSetElementPosition(hudBuildings, &position, 1);
		}
	}*/
	else //if (sound == 113)
	{
		isBlockSlider = false;
	}

	o_onPlayGameSound(_this, sound);
}

void __cdecl minHookFunctions::onPlayCharacterVoice(int sound, character* character, bool b)
{
	o_onPlayCharacterVoice(sound, character, b);
}

void __thiscall minHookFunctions::onHudElementClicked(int* param_1, int param_2, int param_3)
{
	hudBuildings = param_1[0x76];
	if (param_3 != 1)
	{
		o_onHudElementClicked(param_1, param_2, param_3);
		return;
	}

	if (param_2 == param_1[0x7d]) // more   
	{
		if (auto sett = gameHelpers::getGameDataAll()->selectInfo->getSelectedSettlement(); sett /*&& sett->buildingsNum > 24*/)
		{
			arrowClick++;
			coordPair position{ 273, 53 };
			position.yCoord -= 126 * arrowClick;

			o_onSetElementPosition(param_1[0x76], &position, 1);

			double b = ceil(static_cast<double>(sett->buildingsNum) / 12.0);
			double c = static_cast<double>(arrowClick);
			if (c < b - 1)
				return;

			arrowClick     = 0;
			arrowRememberY = position.yCoord;
			arrowChange    = true;
		}
	}

	else if (param_2 == param_1[0x7e]) // less   
	{
		if (auto sett = gameHelpers::getGameDataAll()->selectInfo->getSelectedSettlement(); sett /*&& sett->buildingsNum > 24*/)
		{
			arrowClick++;
			coordPair position{ 273, arrowRememberY };
			position.yCoord += 126 * arrowClick;
			position.yCoord = min(position.yCoord, 53);
	
			o_onSetElementPosition(param_1[0x76], &position, 1);
	
			double b = ceil(static_cast<double>(sett->buildingsNum) / 12.0);
			double c = static_cast<double>(arrowClick);
			if (c < b - 1)
				return;

			arrowClick     = 0;
			arrowRememberY = 53;
		}
	}

	o_onHudElementClicked(param_1, param_2, param_3);

	arrowClick     = 0;
	arrowChange    = false;
}

void __thiscall minHookFunctions::onSetElementPosition(int param_1, coordPair* coords, int param_3)
{
	if (arrowChange)
	{
		coords->yCoord = arrowRememberY;
		arrowChange    = false;
	}
	o_onSetElementPosition(param_1, coords, param_3);
}

advancedSettlementInfoScroll* __thiscall minHookFunctions::onCreateAdvancedSettlementInfoScroll(advancedSettlementInfoScroll* _this, int width, int height)
{
	advancedSettlementInfoScrollClass = o_onCreateAdvancedSettlementInfoScroll(_this, width, height);
	return advancedSettlementInfoScrollClass;
}

void __fastcall minHookFunctions::onResetUnitRecruitAndRetrainScroll(DWORD _this)
{
	unitRecruitAndRetrainScrollClass = _this;
	if (isBlockSlider)
		return;
	o_onResetUnitRecruitAndRetrainScroll(_this);
}

int* __thiscall minHookFunctions::onTraitsListConstructor1(int* _this, int width, int height)
{
	if (height == 92 && height != traitsChildWindowHeight)
	{
		height = traitsChildWindowHeight;
		log("minHookFunctions::onTraitsListConstructor1(set new height: " + to_string(height) + ")");
	}
	return o_onTraitsListConstructor1(_this, width, height);
}

int* __thiscall minHookFunctions::onTraitsListConstructor2(int* _this, int width, int height)
{
	if (height == 92 && height != traitsChildWindowHeight)
	{
		height = traitsChildWindowHeight;
		log("minHookFunctions::onTraitsListConstructor2(set new height: " + to_string(height) + ")");
	}
	return o_onTraitsListConstructor2(_this, width, height);
}

void __fastcall minHookFunctions::onReinitNamedCharacterInfoScroll(int param_1)
{
	if (traitsChildWindowHeight != 92 && *(int*)(*(int*)(param_1 + 0x34c) + 0x48) == *(int*)(param_1 + 0x330))
	{
		isReset = true;
	//	//get address onResetTraitsListHeight
	//	int address = **(int**)(param_1 + 0x33c) + 0x34;
	//	log("minHookFunctions::onReinitNamedCharacterInfoScroll(onResetTraitsListHeight address: " + to_string(address) + ")");
	}
	o_onReinitNamedCharacterInfoScroll(param_1);
}

void __thiscall minHookFunctions::onResetTraitsListHeight(int _this, int height)
{
	if (isReset && height != traitsChildWindowHeight)
	{
		isReset = false;
		height = traitsChildWindowHeight;
	//	log("minHookFunctions::onResetTraitsListHeight(set new height: " + to_string(traitsChildWindowHeight) + ")");
	}
	o_onResetTraitsListHeight(_this, height);
}

void __thiscall minHookFunctions::onMercenaryScrollInit(mercenaryScroll* _this, int param_2, int param_3, int param_4, int param_5, int param_6)
{
	o_onMercenaryScrollInit(_this, param_2, param_3, param_4, param_5, param_6);
	lastMercScroll = _this;
//	log("minHookFunctions::onMercenaryScrollInit(" + pointerToString(_this) + ")");
}

void __thiscall minHookFunctions::onSetSliderState(sliderStruct* _this, int sliderState, char param_3)
{
	lastSliderStruct = _this;
	if (sliderState == 0 || !isMercenaryScrollOpen || !lastMercScroll || lastMercScroll->slider != _this)
	{
		return o_onSetSliderState(_this, sliderState, param_3);
	}

	if (_this->currentState != sliderState && checkActive() && lastMercScroll->slider == _this)
	{
	//	int indent = /*493 - 348*/145 / _this->maxState;
		int indent = 38 * mercsNumOfLine / _this->maxState;
		if (sliderState < _this->currentState)
		{
			_this->yPosThumb -= indent;
			_this->yPosThumb = max(_this->yPosThumb, _this->yPos);
			for (int i = 0; i < lastMercScroll->mercsNum; i++)
			{
				lastMercScroll->mercs[i]->posY += 57;
				bufferPosY[i] = lastMercScroll->mercs[i]->posY;
			}
		}
		else
		{
			_this->yPosThumb += indent;
			_this->yPosThumb = min(_this->yPosThumb, _this->yPos + _this->length);
			for (int i = 0; i < lastMercScroll->mercsNum; i++)
			{
				lastMercScroll->mercs[i]->posY -= 57;
				bufferPosY[i] = lastMercScroll->mercs[i]->posY;
			}
		}
		_this->currentState = sliderState;
	//	log("onSetSliderState: lastMercScroll->mercs[1]->posY: " + to_string(lastMercScroll->mercs[1]->posY));
	}
}

bool minHookFunctions::checkSetPosY(void* pointer)
{
	if (!checkActive() || lastMercScroll->mercsNum <= mercsMaxSlots || lastMercScroll->slider->currentState == 0
		|| bufferPosY.size() < static_cast<size_t>(lastMercScroll->mercsNum))
		return false;

//	log("checkSetPosY");
	for (int i = 0; i < lastMercScroll->mercsNum; i++)
	{
		if (pointer == lastMercScroll->mercs[i])
		{
		//	log("checkSetPosY(i: " + to_string(i) + ", posY: " + to_string(lastMercScroll->mercs[i]->posY) + ")");
			lastMercScroll->mercs[i]->posY = bufferPosY[i];
			return true;
		}
	}
	return false;
}

void __fastcall minHookFunctions::onElementReset(void* param_1)
{
	if (checkSetPosY(param_1))
		return;
	o_onElementReset(param_1);
}

bool __cdecl minHookFunctions::onSetColor(rgba* color)
{
	checkSet(color);
	return o_onSetColor(color);
}

UNICODE_STRING**& __thiscall minHookFunctions::onCreateDebugInfoText(UNICODE_STRING**& _this, const char* str)
{
	UNICODE_STRING**& result = o_onCreateDebugInfoText(_this, str);
	if (isDebugInfoOpen && result)
		debugInfoText = gameStringHelpers::uniStringToStr(result);
	return result;
}

void __thiscall minHookFunctions::onCharacterInfoFrameInit(characterInfoFrame* _this, characterRecord& nc, int param_3, int param_4, int param_5)
{
	o_onCharacterInfoFrameInit(_this, nc, param_3, param_4, param_5);
	log("onCharacterInfoFrameInit: nc: " + string(nc.fullName) + ", portrait: " + string(_this->details->img->texturePath));
	rememberTextures = _this->textures;
}

/////////////////////////
///===== CAMERA ======///
/////////////////////////

void __thiscall minHookFunctions::onCameraMove(void* _this, vector3* camPos, vector3* camTarget, bool some)
{
	//Disable the EOP camera during camera initialization on the tactic map.
	if (stratCamera.isBattle && camTarget->x == 0.0f && camTarget->y == 0.0f && camTarget->z == 0.0f)
	{
		stratCamera.isInit = false;
		return o_onCameraMove(_this, camPos, camTarget, some);
	}

	if (!stratCamera.isInit)
		return o_onCameraMove(_this, camPos, camTarget, some);

	if (ImGui::IsKeyDown(ImGuiKey_Keypad7))
		stratCamera.rotate(_this, -1.0f);
	else if (ImGui::IsKeyDown(ImGuiKey_Keypad9))
		stratCamera.rotate(_this, 1.0f);

	if (ImGui::IsKeyDown(ImGuiKey_KeypadSubtract))
		stratCamera.zoom(1.0f);
	else if (ImGui::IsKeyDown(ImGuiKey_KeypadAdd))
		stratCamera.zoom(-1.0f);

	else if (ImGui::IsKeyDown(ImGuiKey_Keypad8))
		return stratCamera.reset(true);

	if (stratCamera.isRotated)
		return;

	stratCamera.set(*camPos, *camTarget);
	o_onCameraMove(_this, camPos, camTarget, some);
}

void __thiscall minHookFunctions::onCameraAction(void* _this, cameraState action, float speed)
{
	if (!stratCamera.isInit || stratCamera.isBattle)
		return o_onCameraAction(_this, action, speed);

	switch (action)
	{
		case rotateLeft:
		{
			stratCamera.moveX(-1.0f);
			break;
		}
		case rotateRight:
		{
			stratCamera.moveX(1.0f);
			break;
		}
		case rotateUp:
		{
			stratCamera.moveY(1.0f);
			break;
		}
		case rotateDown:
		{
			stratCamera.moveY(-1.0f);
			break;
		}
		case stepLeft:
		{
			stratCamera.moveX(-1.0f);
			break;
		}
		case stepRight:
		{
			stratCamera.moveX(1.0f);
			break;
		}
		case stepFwd:
		{
			stratCamera.moveY(1.0f);
			break;
		}
		case stepBack:
		{
			stratCamera.moveY(-1.0f);
			break;
		}
	}
	o_onCameraAction(_this, action, speed);
}

int* __thiscall minHookFunctions::onAerialMapInit(int* _this, vector3* maxHeightVec, float maxHeight)
{
	int* result = o_onAerialMapInit(_this, maxHeightVec, maxHeight);

	stratCamera.isInit = isEopCameraEnable;
	//When returning from the tactic to the strategy map, set the coordinates that were in effect prior to calling `transitionToBattle`.
	//This will also work if load a save from the tactic (for example, during a naval battle script).
	if (stratCamera.isInit && stratCamera.isBattle)
	{
		stratCamera.isBattle = false;
		stratCamera.reset(false);
		stratCamera.resetPos();
	}
	//When loading a save, the camera resets to the default position, without rotation.
	//But in the future, it might be possible to save the coordinates in the save file and restore them when loading the save.
	else if (stratCamera.isInit)
		stratCamera.reset(false);
	else if (!stratCamera.isInit)
		return result;

	if (stratCamera.camLog)
		log("onAerialMapInit start: result[5]: " + to_string(result[5]) + ", result[4]: " + to_string(result[4]) + ", result[6]: " + to_string(result[6]) + ", result[7]: " + to_string(result[7]) + ", result[8]: " + to_string(result[8]) + ", result[9]: " + to_string(result[9]));

	//Slightly increase the rendering area of aerialMap.
	result[5] *= 1.5;
	result[4] *= 1.5;
	result[6] *= 1.5;
	result[7] *= 1.5;
	result[8] *= 1.5;
	result[9] *= 1.5;

	if (stratCamera.camLog)
		log("onAerialMapInit end: result[5]: " + to_string(result[5]) + ", result[4]: " + to_string(result[4]) + ", result[6]: " + to_string(result[6]) + ", result[7]: " + to_string(result[7]) + ", result[8]: " + to_string(result[8]) + ", result[9]: " + to_string(result[9]));

	return result;
}

bool __thiscall minHookFunctions::onAerialMapUpdate(int _this, bool full)
{
	if (stratCamera.isInit && (stratCamera.isMove || stratCamera.isRotated))
	{
		//The camera coordinates and the AerialMap need to match.
		coordPair coords = stratCamera.getAerialMapXY();
		*(int*)(_this + 0x11bc) = coords.xCoord; //aerialMapX
		*(int*)(_this + 0x11c0) = coords.yCoord; //aerialMapY
	}
	return o_onAerialMapUpdate(_this, full);
}

//Lua, script commands: M2TW.stratMap.jumpCamera, "snap_strat_camera"
void __thiscall minHookFunctions::onAerialMapSetPosition(int _this, int x, int y)
{
	o_onAerialMapSetPosition(_this, x, y);
	//Update the camera coordinates when the AerialMap coordinates change (for example, upon clicking the minimap).
	stratCamera.setPosition(x, y);
}

//Lua, script commands: M2TW.stratMap.moveCamera, "move_strat_camera"
void __cdecl minHookFunctions::onMoveStratCameraSlow(int x, int y)
{
	if (!stratCamera.isInit)
		return o_onMoveStratCameraSlow(x, y);
	stratCamera.targetX   = static_cast<float>(x);
	stratCamera.targetY   = static_cast<float>(y);
	stratCamera.isMove    = true;
	stratCamera.isRotated = false;
}

//Lua, script, console commands: M2TW.stratMap.zoomCamera, "zoom_strat_camera", "zoom"
void __cdecl minHookFunctions::onZoomStratCamera(float target)
{
	if (!stratCamera.isInit)
		return o_onZoomStratCamera(target);
	stratCamera.targetZ   = min(max(target, 0.2f), 1.08f);
	stratCamera.isMove    = true;
	stratCamera.isRotated = false;
}

void minHookFunctions::stratCameraMouseWheel()
{
	if (!stratCamera.isInit || stratCamera.isBattle || !stratMapHelpers::isStratMap() || graphicsD3D::dataS.ifMouseOrKeyBoardAtImgui)
		return;

	if (ImGui::GetIO().MouseWheel > 0.0f)
	{
		stratCamera.zoom(-2.0f);
	}
	else if (ImGui::GetIO().MouseWheel < 0.0f)
	{
		stratCamera.zoom(2.0f);
	}
}

void minHookFunctions::enableEopCamera(bool set, float speedRotate, float speedMove, float speedZoom, bool toBattle)
{
	isEopCameraEnable       = set;
	stratCamera.speedRotate = speedRotate;
	stratCamera.speedMove   = speedMove;
	stratCamera.speedZoom   = speedZoom;
	stratCamera.toBattle    = toBattle;
	if (stratMapHelpers::isStratMap())
		stratCamera.isInit = set;
}

void minHookFunctions::transitionToBattle(const int x, const int y)
{
	if (!isEopCameraEnable)
		return;
	stratCamera.transitionToBattle();
	stratCamera.setPosition(x, y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// CALL GAME FUNCTIONS /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

characterRecord* minHookFunctions::createWife(characterRecord* husband)
{
	if (!husband->isMale || husband->spouse)
	{
		log("characterRecord::createMessageMarriageProposal: son is already married");
		return nullptr;
	}

	characterRecord* new_wife = onCreateWife(husband);
	marriageOption* mo = techFuncs::createGameClass<marriageOption>();
	GAME_FUNC(void(__thiscall*)(marriageOption*, characterRecord*, characterRecord*), marriageOptionClassConstructor)(mo, new_wife, husband);
	onCreateMessageAboutMarriage(husband, new_wife, mo); 

	return new_wife;
}

characterRecord* minHookFunctions::createHusband(characterRecord* daughter)
{
	if (daughter->isMale || daughter->spouse)
	{
		log("characterRecord::createMessageMarriageProposal: daughter is already married");
		return nullptr;
	}

	characterRecord* new_husband = onCreateCandidateMarrying(daughter);
	marriageOption* mo = techFuncs::createGameClass<marriageOption>();
	GAME_FUNC(void(__thiscall*)(marriageOption*, characterRecord*, characterRecord*), marriageOptionClassConstructor)(mo, new_husband, daughter);
	onDaughterReadyMarryHusband(daughter, new_husband, mo);

	return new_husband;
}

void minHookFunctions::resetTooltipText(const char* text)
{
	if (!lastTooltipClass1 || !lastTooltipClass2 || !lastTooltipUniString)
		return;

	gameStringHelpers::createUniString(*lastTooltipUniString, text);
	GAME_FUNC(void(__thiscall*)(DWORD, void*, UNICODE_STRING**&), resetTooltipTextAdd)(lastTooltipClass1, lastTooltipClass2, *lastTooltipUniString);
}

void minHookFunctions::createBlockadePortMessage(armyStruct* fleet, settlementStruct* settlement)
{
	GAME_FUNC(void(__cdecl*)(armyStruct*, settlementStruct*), createBlockadePortMessageAdd)(fleet, settlement);
}

void minHookFunctions::setAdvancedSettlementInfoScrollFrameSize(int width, int height)
{
	if (!advancedSettlementInfoScrollClass || !advancedSettlementInfoScrollClass->elements || !advancedSettlementInfoScrollClass->elements->frame)
		return;
	advancedSettlementInfoScrollClass->elements->frame->width  = width;
	advancedSettlementInfoScrollClass->elements->frame->height = height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////// TESTS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Fix broken army:   
//In my mod, I encountered an issue where the Scottish army was listed in the slave' army array, causing a fatal crash. 
//This fix solved the problem for me. I'll leave this here for now�it might come in handy�and delete it later.
static bool checkFactionArmy(armyStruct* targetArmy, int x, int y)
{
	campaign* camp = gameHelpers::getGameDataAll()->campaignStruct;
	for (int f = 0; f < camp->factionCount; f++)
	{
		factionStruct* faction = camp->factionsSortedByTurnOrder[f];
		for (int a = 0; a < faction->stackNum; a++)
		{
			armyStruct* army = faction->armies[a];
			if (targetArmy == army && targetArmy->faction == faction)
			{
				return true;
			}
			else if (targetArmy == army && targetArmy->faction != faction)
			{
				minHookFunctions::log(
					"checkFactionArmy: army fix start(coords: " + to_string(x) + "," + to_string(y) + 
					" | real army faction: " + string(targetArmy->faction->factionRecord->facName)  + ", stackNum: " + to_string(targetArmy->faction->stackNum) + 
					" | bug army faction: "  + string(faction->factionRecord->facName)              + ", stackNum: " + to_string(faction->stackNum)             +
				")");

				GAME_FUNC_RAW(void(__thiscall*)(factionStruct*, armyStruct*, factionStruct*), 0x004f4520)(faction, targetArmy, targetArmy->faction);//armySwitchFaction //disk   

				minHookFunctions::log(
					"checkFactionArmy: army fix end(coords: " + to_string(x) + "," + to_string(y) + 
					" | real army faction: " + string(targetArmy->faction->factionRecord->facName)  + ", stackNum: " + to_string(targetArmy->faction->stackNum) + 
					" | bug army faction: "  + string(faction->factionRecord->facName)              + ", stackNum: " + to_string(faction->stackNum)             +
				")");

				return true;
			}
		}
	}
	return false;
}
static void checkRegionsArmies()
{
	stratMap* map = stratMapHelpers::getStratMap();
	if (!map || !map->isOpen)
		return;

	for (int r = 0; r < map->regionsNum; r++)
	{
		auto region = &map->regions[r];

		for (int a = 0; a < region->stacksNum; a++)
		{
			armyStruct* army = region->armies[a];

			int x = army->getX(); 
			int y = army->getY();
			if (army && !checkFactionArmy(army, x, y))
			{
				minHookFunctions::log("checkRegionsArmies: army not found(coords: " + to_string(x) + "," + to_string(y) + " | army: " + minHookFunctions::pointerToString(army) + ", | faction->stackNum: " + to_string(army->faction->stackNum) + ")");
			}
		}

	}
}

int buffer_sound = 0;
int buffer_line = 3;
//### Don't pay attention to this mess, I'll finish some things and clean this up a bit later. (^_^)   
void minHookFunctions::draw()
{
	if (!openConsole)
		return;

	/////////////////////////////////////////////////////////////////////////////////
	ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(0.5f, 0.5f), ImGuiCond_Once, ImVec2(0.0f, 0.0f));
	ImGui::Begin("consoleMinHook", nullptr, ImGuiWindowFlags_None);


	if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && selectTile)
	{
		rememberCoords.xCoord = selectTile->getTileX();
		rememberCoords.yCoord = selectTile->getTileY();
	}
	ImGui::Separator();
	if (lastSliderStruct)
	{
		ImGui::Text(("cursorCoords:   " + to_string(lastSliderStruct->cursorPosX) + ", " + to_string(lastSliderStruct->cursorPosY)).c_str());
	}
	if (selectTile)
	{
		ImGui::Text(("selectCoords:   " + to_string(selectTile->getTileX()) + ", " + to_string(selectTile->getTileY())).c_str());
		ImGui::Text(("rememberCoords: " + to_string(rememberCoords.xCoord) + ", " + to_string(rememberCoords.yCoord)).c_str());
	}
	ImGui::Separator();


	/////////////////////////////////////////////////////////////////////////////////

	if (ImGui::Button("moveWatchtower") && selectTile)
	{
		if (auto wt = selectTile->getWatchtower(); wt)
		{
			GAME_FUNC(void(__thiscall*)(watchTowerStruct*, bool, factionStruct*, int),
				changeWatchTowerFaction)(wt, 0, gameHelpers::getGameDataAll()->campaignStruct->getSlaveFaction(), 0);

			coordPair coords{ 448, 217 };
			GAME_FUNC_RAW(void(__thiscall*)(stratPathFinding*, void*, coordPair*), 0x004cd440)(campaignHelpers::getStratPathFinding(), wt, &coords);//disk

			wt->regionID = stratMapHelpers::getTile(448, 217)->regionId;

			wt->faction->tilesFac->updateFromObject(wt);
		}
	}
	if (ImGui::Button("moveSettlement"))
	{
		if (auto sett = gameHelpers::getGameDataAll()->selectInfo->getSelectedSettlement(); sett)
		{
			int xCoord = rememberCoords.xCoord;
			int yCoord = rememberCoords.yCoord;

			coordPair coords{ xCoord, yCoord };
			GAME_FUNC_RAW(void(__thiscall*)(stratPathFinding*, void*, coordPair*), 0x004cd440)(campaignHelpers::getStratPathFinding(), sett, &coords);//disk

			GAME_FUNC(void(__thiscall*)(stratPathFinding*, settlementStruct*), areaOfInfluence)(campaignHelpers::getStratPathFinding(), sett);

			GAME_FUNC(char(__thiscall*)(settlementStruct*), residenceTileCharacterCheck)(sett);

			sett->recalculate(true);

			sett->rallyPointX = xCoord;
			sett->rallyPointY = yCoord;

			sett->regionID = stratMapHelpers::getTile(xCoord, yCoord)->regionId;

			sett->faction->tilesFac->updateFromObject(sett);
		}
	}
	if (ImGui::Button("moveFort"))
	{
		if (auto fort = gameHelpers::getGameDataAll()->selectInfo->getSelectedFort(); fort)
		{
			int xCoord = rememberCoords.xCoord;
			int yCoord = rememberCoords.yCoord;

			coordPair coords{ xCoord, yCoord };
			GAME_FUNC_RAW(void(__thiscall*)(stratPathFinding*, void*, coordPair*), 0x004cd440)(campaignHelpers::getStratPathFinding(), fort, &coords);//disk

			fort->regionID = stratMapHelpers::getTile(xCoord, yCoord)->regionId;

			fort->faction->tilesFac->updateFromObject(fort);
		}
	}
	if (ImGui::Button("movePort") && selectTile)
	{
		if (auto port = selectTile->getPort(); port)
		{
			//port
			int xCoord = rememberCoords.xCoord;
			int yCoord = rememberCoords.yCoord;

			int portDockDifferenceX = port->xCoord - port->dockX;
			int portDockDifferenceY = port->yCoord - port->dockY;

			coordPair coords{ xCoord, yCoord };
			GAME_FUNC_RAW(void(__thiscall*)(stratPathFinding*, void*, coordPair*), 0x004cd440)(campaignHelpers::getStratPathFinding(), port, &coords);//disk

			port->rallyCoordX = xCoord;
			port->rallyCoordY = yCoord;

			port->dockX = xCoord + portDockDifferenceX;
			port->dockY = yCoord + portDockDifferenceY;

		//	port->regionID = stratMapHelpers::getTile(xCoord, yCoord)->regionId;

			port->fac->tilesFac->updateFromObject(port);


			//dock
			coordPair coordsD{ port->dockX, port->dockY };
			GAME_FUNC_RAW(void(__thiscall*)(stratPathFinding*, void*, coordPair*), 0x004cd440)(campaignHelpers::getStratPathFinding(), port->portDock, &coordsD);//disk
		
			port->portDock->rallyCoordX = port->dockX;
			port->portDock->rallyCoordY = port->dockY;
		
			port->portDock->dockX = port->dockX;
			port->portDock->dockY = port->dockY;
		
		//	port->portDock->regionID = stratMapHelpers::getTile(xCoord, yCoord)->regionId;
		
			port->portDock->fac->tilesFac->updateFromObject(port);
		}
	}
	if (ImGui::Button("moveResource") && selectTile)
	{
		if (auto resource = selectTile->getResource(); resource)
		{
			int xCoord = rememberCoords.xCoord;
			int yCoord = rememberCoords.yCoord;

			coordPair coords{ xCoord, yCoord };
			GAME_FUNC_RAW(void(__thiscall*)(stratPathFinding*, void*, coordPair*), 0x004cd440)(campaignHelpers::getStratPathFinding(), resource, &coords);//disk

			resource->regionID = stratMapHelpers::getTile(xCoord, yCoord)->regionId;
		}
	}
	if (ImGui::Button("damageWall"))
	{
		if (auto sett = gameHelpers::getGameDataAll()->selectInfo->getSelectedSettlement(); sett)
		{
			GAME_FUNC_RAW(void(__thiscall*)(settlementStruct*, int /*breaches*/, bool /*gatehouse*/), 0x005edd00)(sett, 0, true);//disk
		}
	}
	if (ImGui::Button("createBlockadePortMessage"))
	{
		if (auto sett = gameHelpers::getGameDataAll()->selectInfo->getSelectedSettlement(); sett)
		{
			if (auto fleet = stratMapHelpers::getTile(rememberCoords.xCoord, rememberCoords.yCoord)->getCharacter(); fleet && fleet->army)
			{
			//	blockadePort(fleet, sett->port);
				createBlockadePortMessage(fleet->army, sett);
			}
		}
	}
	if (ImGui::Button("blockadePort"))
	{
		if (auto sett = gameHelpers::getGameDataAll()->selectInfo->getSelectedSettlement(); sett && sett->port)
		{
			if (auto fleet = stratMapHelpers::getTile(rememberCoords.xCoord, rememberCoords.yCoord)->getCharacter(); fleet && fleet->army)
			{
				GAME_FUNC_RAW(void(__thiscall*)(portBuildingStruct*, bool, factionStruct*, armyStruct*), 0x004c1a20)(sett->port, false, fleet->army->faction, fleet->army);//disk
			}
		}
	}
	ImGui::InputInt("sound", &buffer_sound);
	if (ImGui::Button("playCharacterVoice"))
	{
		if (auto character = selectTile->getCharacter(); character)
		{
			onPlayCharacterVoice(buffer_sound, character, false);
		}
	}
	ImGui::InputInt("line", &buffer_line);
	if (ImGui::Button("setMercenariesNumberOfLines"))
	{
		gameHelpers::setMercsScrollNumOfLines(buffer_line);
	}
	if (ImGui::Button("setMercsScrollParameters"))
	{
		gameHelpers::setMercsScrollParameters(14, 7, 4, 2, 142);
	}
	if (ImGui::Button("FIX/kill checkRegionsArmies()"))
	{
		checkRegionsArmies();
	}
	if (rememberTextures)
	{
		ImGui::Image(rememberTextures->getGameTexture(1), ImVec2(500, 500));
	//	ImGui::Image(rememberTextures->getGameTexture(2), ImVec2(500, 500));
	//	ImGui::Image(rememberTextures->getGameTexture(3), ImVec2(500, 500));
	//	ImGui::Image(rememberTextures->getGameTexture(4), ImVec2(500, 500));
	//	ImGui::Image(rememberTextures->getGameTexture(5), ImVec2(500, 500));
	}

	ImGui::Checkbox("stratCamera.camLog", &stratCamera.camLog);
	ImGui::Checkbox("stratCamera.isInit", &stratCamera.isInit);
	ImGui::Checkbox("isEopCameraEnable" , &isEopCameraEnable);

	/////////////////////////////////////////////////////////////////////////////////

	ImGui::End();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////








