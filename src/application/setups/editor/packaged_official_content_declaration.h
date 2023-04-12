#pragma once

struct packaged_official_content;

struct editor_resource_pools;
struct editor_official_resource_map;

const editor_resource_pools& official_get_resources(const packaged_official_content&);
const editor_official_resource_map& official_get_resource_map(const packaged_official_content&);
