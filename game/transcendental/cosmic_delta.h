#pragma once

namespace RakNet {
	class BitStream;
}

class cosmos;

class cosmic_delta {
public:
	static void encode(const cosmos& base, const cosmos& encoded, RakNet::BitStream& to);
	static void decode(cosmos& into, RakNet::BitStream& from, bool resubstantiate_partially = false);
};