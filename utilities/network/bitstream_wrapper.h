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
			std::string next_property;
			
			RakNet::BitStream& get_stream() {
				return stream;
			}

			void name_property(std::string str) {
				next_property = str;
			}
			
			std::string get_next_property() {
				auto copy = next_property;
				next_property.clear();
				return copy;
			}

			template <class T>
			void Write(T const& in) {
				stream.Write(in);
				std::stringstream instr;

				instr << typeid(T).name() << " " << get_next_property() << " = " << in << std::endl;
				content += instr.str();
			}

			template <>
			void Write(unsigned char const& in) {
				stream.Write(in);
				std::stringstream instr;

				instr << typeid(unsigned char).name() << " " << get_next_property() << " = " << int(in) << std::endl;
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

				instr << typeid(T).name() << " " << get_next_property() << " = " << in.x << '\t' << in.y << std::endl;
				content += instr.str();
			}
			
			void WriteString(const std::string& str) {
				stream.Write(str.c_str());
				content += "std::string ";
				content += get_next_property() + ':';
				content += str;
				content += '\n';
			}

			void WriteBitstream(bitstream& other) {
				if (other.stream.GetNumberOfBitsUsed() > 0) {
					stream.WriteBits(other.stream.GetData(), other.stream.GetNumberOfBitsUsed(), false);

					content += '\n' + get_next_property() + std::string(" { \n") + other.content;
					content += "\n}";
				}
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