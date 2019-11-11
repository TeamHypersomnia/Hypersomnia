#pragma once
#include <cstdint>
#include <string>

uint64_t portable_hash(float x);
uint64_t portable_hash(const std::string& x);
uint64_t portable_hash(uint64_t x);
uint64_t portable_hash(uint32_t x);
uint64_t portable_hash(int32_t x);

