#pragma once
#include <variant>
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "application/masterserver/server_heartbeat.h"
#include "application/masterserver/gameserver_commands.h"
#include "application/masterserver/masterserver_commands.h"
#include "work_result.h"

struct config_json_table;
struct server_heartbeat;

work_result perform_masterserver(const config_json_table&);
