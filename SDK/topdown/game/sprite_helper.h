#pragma once
#include "texture_baker/texture_baker.h"
#include "../renderable.h"
#include <string>

struct sprite_helper : sprite {
	augmentations::texture_baker::texture tex;
	augmentations::texture_baker::image img;

	sprite_helper(std::wstring filename, augmentations::texture_baker::atlas& atl);
};