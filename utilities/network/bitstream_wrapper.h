#pragma once
#include <string>
#include <sstream>

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"

namespace augs {
	namespace network {
		struct bitstream {
			RakNet::BitStream stream;
			std::string content;
			
			RakNet::BitStream& get_stream() {
				return stream;
			}

			template <class T>
			void Write(T const& in) {
				stream.Write(in);
				std::stringstream instr;

				instr << typeid(T).name() << ": " << in << std::endl;
				content += instr.str();
			}

			template <class T>
			void WritePOD(T in) {
				Write<T>(in);
			}

			template <class T>
			void WriteVec(T const& in) {
				stream.Write(in);
				std::stringstream instr;

				instr << typeid(T).name() << ": " << in.x << '\t' << in.y << std::endl;
				content += instr.str();
			}

			void WriteString(const std::string& str) {
				stream.Write(str.c_str());
				content += "std::string: ";
				content += str;
				content += '\n';
			}

			void WriteBitstream(bitstream& other) {
				stream.WriteBits(other.stream.GetData(), other.stream.GetNumberOfBitsUsed(), false);
				content += other.content;
			}

			void WriteGuid(RakNet::RakNetGUID& guid) {
				Write(guid.g);
			}

			template <class T>
			bool Read(T& out) {
				return stream.Read(out);
			}

			RakNet::RakNetGUID ReadGuid() {
				RakNet::RakNetGUID guid;

				decltype(guid.g) my_id;
				stream.Read(my_id);

				return RakNet::RakNetGUID(my_id);
			}

		
			template <class T>
			T ReadPOD() {
				T data;
				stream.Read<T>(data);
				return data;
			}

			void IgnoreBytes(unsigned int n) {
				return stream.IgnoreBytes(n);
			}

			RakNet::BitSize_t GetNumberOfBitsUsed() {
				return stream.GetNumberOfBitsUsed();
			}

			void Reset() {
				content = std::string();
				return stream.Reset();
			}

			void ResetReadPointer() {
				return stream.ResetReadPointer();
			}

			void SetReadOffset(RakNet::BitSize_t n) {
				return stream.SetReadOffset(n);
			}

			RakNet::BitSize_t GetReadOffset() {
				return stream.GetReadOffset();
			}

			RakNet::BitSize_t GetNumberOfUnreadBits() {
				return stream.GetNumberOfUnreadBits();
			}
		};
	}
}