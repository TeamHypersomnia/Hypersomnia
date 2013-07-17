#pragma once
#include "config.h"
#include <locale>

using namespace std;
namespace augmentations {
	namespace config {
		input_file::property::operator int() const {
			return int_val;
		}

		input_file::property::operator float() const {
			return float_val;
		}

		input_file::property::operator double() const {
			return double_val;
		}

		input_file::property::operator wstring() const {
			return string_val;
		}

		input_file::property::operator input_file::color() const {
			int r, g, b, a;
			swscanf_s(string_val.c_str(), L"rgba(%d,%d,%d,%d)", &r, &g, &b, &a);
			return color(r, g, b, a);
		}

		input_file::input_file() {
		}

		input_file::input_file(const char* filename) {
			open(filename);
		}

		void input_file::open(const char* filename) {
			locale::global(locale(""));
			wifstream in(filename);
			const int CNT = 1000;
			wchar_t buf[CNT]; 
			while(in.good()) {
				wstring name;
				property prop;
				in >> prop._type;
				in >> name;

				wchar_t w;
				in.get(w);

				if(prop._type == L"string")		{ 
					in.getline(buf, CNT); 
					prop.string_val = wstring(buf);
				}
				else if(prop._type == L"int")		{ in >> prop.int_val;	 }
				else if(prop._type == L"float")		{ in >> prop.float_val;	 }
				else if(prop._type == L"double")	{ in >> prop.double_val; }
				else {
					auto func = callbacks.find(prop._type);
					if(func != callbacks.end())
						(*func).second.first(in, prop);
					else {
						in.getline(buf, CNT); 
						prop.string_val = wstring(buf);
					}
				}

				values[name] = prop;
			}
			in.close();
		}

		void input_file::save(const char* filename) {
			wofstream out(filename);

			for(auto it = values.begin(); it != values.end(); ++it) {
				out << (*it).second._type;
				out << L' ';
				out << (*it).first;
				out << L' ';
				auto t = (*it).second._type;

				if(t == L"string") out << (*it).second.string_val;
				else if(t == L"int") out << (*it).second.int_val;	
				else if(t == L"float") out << (*it).second.float_val;	
				else if(t == L"double") out << (*it).second.double_val;
				else {
					auto func = callbacks.find(t);
					if(func != callbacks.end())
						(*func).second.second(out, (*it).second);
				}

				out << endl;
			}
			out.close();
		}

		input_file::property& input_file::operator[](wstring name) {
			return values[name];
		}
	}
}