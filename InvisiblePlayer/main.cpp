#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <Nlohmann/json.hpp>
#include <MC/PlayerListPacket.hpp>
#include <MC/BinaryStream.hpp>
#include <MC/Level.hpp>
#include <mc/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/CrashDumpKeyValueData.hpp>
#include <MC/CrashDumpLog.hpp>
#include <MC/LevelStorage.hpp>
#include <LLAPI.h>
#include <EventAPI.h>
Logger logger("InvisiblePlayer");
enum CrashDumpLogStringID;

#define TRJ(key,val)                                         \
if (json.find(key) != json.end()) {                          \
    const nlohmann::json& out = json.at(key);                \
    out.get_to(val);}                                         \


namespace Setting {
	std::list<string> InvPlayers;
	
	nlohmann::json globaljson() {
		nlohmann::json json;
		json["PlayerList"] = InvPlayers;

		return json;
	}
	void initjson(nlohmann::json json) {
		TRJ("PlayerList", InvPlayers);

	}
	void WriteDefaultConfig(const std::string& fileName) {
		std::ofstream file(fileName);
		if (!file.is_open()) {
			std::cout << "Can't open file " << fileName << std::endl;
			return;
		}
		auto json = globaljson();
		file << json.dump(4);
		file.close();
	}
	void LoadConfigFromJson(const std::string& fileName) {
		std::ifstream file(fileName);
		if (!file.is_open()) {
			std::cout << "Can't open file " << fileName << std::endl;
			return;
		}
		nlohmann::json json;
		file >> json;
		file.close();
		initjson(json);
		WriteDefaultConfig(fileName);
	}
}

bool isInvisiblePlayer(Player* player) {
	if (!player->isPlayer()) {
		return false;
	}
	auto name = player->getRealName();
	for (auto& i : Setting::InvPlayers) {
		if (i == name) {
			return true;
		}
	}
	return false;
}

bool isInvisiblePlayer(string name) {
	if (!Level::getPlayer(name)->isPlayer()) {
		return false;
	}
	for (auto& i : Setting::InvPlayers) {
		if (i == name) {
			return true;
		}
	}
	return false;
}


THook(void, "?onReady_ClientGeneration@ServerNetworkHandler@@QEAAXAEAVPlayer@@AEBVNetworkIdentifier@@@Z", ServerNetworkHandler* a1,
	Player* a2,
	const struct NetworkIdentifier* a3) {
	if (isInvisiblePlayer(a2)) {
		return;
	}
	return original(a1, a2, a3);
}


THook(__int64, "?emplace@PlayerListPacket@@QEAAX$$QEAVPlayerListEntry@@@Z", PlayerListPacket* a1, PlayerListEntry& a2) {
	if (a1->type == PlayerListPacketType::Add) {
		if (isInvisiblePlayer(a2.name)) {
			return 0;
		}
	}
	return original(a1, a2);
}
bool islist = false;

THook(int, "?getUserCount@Level@@UEBAHXZ", Level* a1) {
	int count = original(a1);
	if(islist) return count - 1;
	return count;
}


THook(void, "?_onPlayerLeft@ServerNetworkHandler@@AEAAXPEAVServerPlayer@@_N@Z", ServerNetworkHandler* a1, ServerPlayer* a2, char a3) {
	if (isInvisiblePlayer(a2)) {
		auto cert = a2->getCertificate();
		if (cert) {
			auto& UniqueID = a2->getUniqueID();
			//auto v29 = CrashDumpKeyValueData((CrashDumpLogStringID)75,(CrashDumpLogStringID)0,UniqueID.get(),0);
			//CrashDumpLog::logKeyValue(v29);
			Global<LevelStorage>->save(*a2);
			a2->disconnect();
			a2->remove();
		}
		return;
	}
	return original(a1, a2, a3);
}

THook(void, "?execute@ListCommand@@UEBAXAEBVCommandOrigin@@AEAVCommandOutput@@@Z", void* a1, void* a2, void* a3) {
	Level::forEachPlayer([](Player& sp)->bool {
		if (isInvisiblePlayer(&sp)) {
			islist = true;
		}
		return true;
		});
	original(a1,a2, a3);
	islist = false;
}

//
THook(bool, "std::_Func_impl_no_alloc<<lambda_ac82dc6e67a6393036f6332014eafe80>,bool,Player &>::_Do_call", __int64 a1,
	Player* a2) {
	if (isInvisiblePlayer(a2)) {
		return true;
	}
	return original(a1, a2);
}


THook(bool, "std::_Func_impl_no_alloc<<lambda_f326310c15e76ef408de41745bd1cc1a>,bool,Player &>::_Do_call", __int64 a1,
	Player* a2) {
	if (isInvisiblePlayer(a2)) {
		return true;
	}
	return original(a1, a2);
}


void loadCfg() {
	//config
	if (!std::filesystem::exists("plugins/InvisiblePlayer"))
		std::filesystem::create_directories("plugins/InvisiblePlayer");
	//tr	
	if (std::filesystem::exists("plugins/InvisiblePlayer/config.json")) {
		try {
			Setting::LoadConfigFromJson("plugins/InvisiblePlayer/config.json");
		}
		catch (std::exception& e) {
			logger.error("Config File isInvalid, Err {}", e.what());
			Sleep(1000 * 100);
			exit(1);
		}
		catch (...) {
			logger.error("Config File isInvalid");
			Sleep(1000 * 100);
			exit(1);
		}
	}
	else {
		logger.info("Config with default values created");
		Setting::WriteDefaultConfig("plugins/InvisiblePlayer/config.json");
	}
}//º”‘ÿ≈‰÷√Œƒº˛


void PluginInit()
{
	loadCfg();
	logger.info("[InvisiblePlayer] Loaded success");
}