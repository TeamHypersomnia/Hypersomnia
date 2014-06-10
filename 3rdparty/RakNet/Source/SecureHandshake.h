/// \file
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#ifndef SECURE_HANDSHAKE_H
#define SECURE_HANDSHAKE_H

#include "NativeFeatureIncludes.h"

#if LIBCAT_SECURITY==1

// If building a RakNet DLL, be sure to tweak the CAT_EXPORT macro meaning
#if !defined(_RAKNET_LIB) && defined(_RAKNET_DLL)
# define CAT_BUILD_DLL
#else
# define CAT_NEUTER_EXPORT
#endif

// Include DependentExtensions in your path to include this
#include "cat/AllTunnel.hpp"

#endif // LIBCAT_SECURITY

#endif // SECURE_HANDSHAKE_H
