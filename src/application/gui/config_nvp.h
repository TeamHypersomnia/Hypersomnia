#pragma once

#define CONFIG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), config.x
#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x
