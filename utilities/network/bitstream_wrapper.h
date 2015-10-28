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
			std::string read_report;
			std::string next_property;
			
			bitstream& operator=(const bitstream& other) {
				stream.Reset();
				WriteBitstream(other);
				SetReadOffset(other.GetReadOffset());

				content = other.content;
				read_report = other.read_report;
				next_property = other.next_property;

				return *this;
			}

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
			void WriteVec(T in) {
				stream.Write(in);
				std::stringstream instr;

				instr << typeid(T).name() << " " << get_next_property() << " = " << in.x << '\t' << in.y << std::endl;
				content += instr.str();
			}
			
			void WriteString(const std::string& str) {
				stream.Write<unsigned short>(static_cast<unsigned short>(str.length()));
				
				for (auto& c : str) {
					stream.Write<char>(c);
				}

				content += "std::string ";
				content += get_next_property() + ':';
				content += str;
				content += '\n';
			}

			std::string ReadString() {
				std::string out;
				out.resize(ReadPOD<unsigned short>());

				for (auto& c : out) {
					c = ReadPOD<char>();
				}

				return out;
			}

			void WriteBits(bitstream& other, size_t n) {
				stream.Write(other.stream, n);

				content += '\n' + get_next_property() + std::string(" { \n") + other.content;
				content += "\n}";
			}

			void WriteBitstream(const bitstream& other) {
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
				auto result = stream.Read(out);
				
				std::stringstream instr;

				if (result) {
					instr << typeid(T).name() << " " << get_next_property() << " = " << out << std::endl;
				}
				else {
					instr << "Couldn't read " << typeid(T).name() << " " << get_next_property() << std::endl;
				}

				read_report += instr.str();

				return result;
			}

			template <>
			bool Read(unsigned char& out) {
				auto result = stream.Read(out);

				std::stringstream instr;

				if (result) {
					instr << typeid(unsigned char).name() << " " << get_next_property() << " = " << int(out) << std::endl;
				}
				else {
					instr << "Couldn't read " << typeid(unsigned char).name() << " " << get_next_property() << std::endl;
				}

				read_report += instr.str();

				return result;
			}

			template <class T>
			T ReadVec() {
				T out;
				auto result = stream.Read(out);

				std::stringstream instr;

				if (result) {
					instr << typeid(T).name() << " " << get_next_property() << " = " << out.x << '\t' << out.y << std::endl;
				}
				else {
					instr << "Couldn't read " << typeid(T).name() << " " << get_next_property() << std::endl;
				}

				read_report += instr.str();

				return out;
			}

			RakNet::RakNetGUID ReadGuid() {
				RakNet::RakNetGUID guid;

				decltype(guid.g) my_id;
				Read(my_id);

				return RakNet::RakNetGUID(my_id);
			}

		
			template <class T>
			T ReadPOD() {
				T data;
				Read(data);
				return data;
			}

			void IgnoreBytes(unsigned int n) {
				std::stringstream instr;
				instr << "Ignored " << n << " bytes.\n";
				read_report += instr.str();

				return stream.IgnoreBytes(n);
			}

			void IgnoreBits(unsigned int n) {
				std::stringstream instr;
				instr << "Ignored " << n << " bits.\n";
				read_report += instr.str();

				return stream.IgnoreBits(n);
			}

			RakNet::BitSize_t GetNumberOfBitsUsed() const {
				return stream.GetNumberOfBitsUsed();
			}

			void Reset() {
				content = std::string();
				read_report = std::string();
				return stream.Reset();
			}

			void ResetReadPointer() {
				return stream.ResetReadPointer();
			}

			void SetReadOffset(RakNet::BitSize_t n) {
				return stream.SetReadOffset(n);
			}

			RakNet::BitSize_t GetReadOffset() const {
				return stream.GetReadOffset();
			}

			RakNet::BitSize_t GetNumberOfUnreadBits() const {
				return stream.GetNumberOfUnreadBits();
			}

			RakNet::BitSize_t GetNumberOfAllBits() const {
				return GetNumberOfUnreadBits() + GetReadOffset();
			}
		};
	}
}