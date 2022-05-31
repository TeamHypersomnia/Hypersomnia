#pragma once
#include "augs/filesystem/path_declaration.h"

#define CA_CERT_PATH (((augs::path_type(DETAIL_DIR) / "web") / "ca-bundle.crt").string())

