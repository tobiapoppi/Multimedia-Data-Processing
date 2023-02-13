#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>

using namespace std;

void error(const char* msg) {
	std::cout << msg << '\n';
	exit(EXIT_FAILURE);
}

void FitCRC_Get16(uint16_t& crc, uint8_t byte)
{
	static const uint16_t crc_table[16] =
	{
		0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
	};
	uint16_t tmp;
	// compute checksum of lower four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[byte & 0xF];
	// now compute checksum of upper four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[(byte >> 4) & 0xF];
}

struct fitHeader {
	uint8_t hsize;
	uint8_t prot_v;
	uint16_t prof_v;
	uint32_t data_size;
	uint8_t data_type[4];
};

struct field {
	uint8_t number;
	uint8_t size;
	uint8_t base_type;
};

struct definitionMessage {
	uint8_t reserved = 0;
	uint8_t arch; //0 LE, 1 BE;
	uint16_t GlobMessageNumber;
	uint8_t numFields;
	std::vector<field> fieldDefinition;
};

struct fieldData {
	size_t num = 0;
	size_t value = 0;
};

void updateCrc(std::istream& is, uint32_t nbytes, uint16_t& crc) {
	
	is.seekg(uint32_t(is.tellg()) - nbytes);
	for (size_t i = 0; i < nbytes; ++i) {
		uint8_t b;
		is.read(reinterpret_cast<char*>(&b), 1);
		FitCRC_Get16(crc, b);
	}
}

void readHeader(istream& is) {
	uint16_t checkCRC = 0;
	uint16_t CRC;
	fitHeader header;

	//1) leggi l'header e controlla il crc
	is.read(reinterpret_cast<char*>(&header), sizeof(fitHeader));
	updateCrc(is, header.hsize - 2, checkCRC);
	is.read(reinterpret_cast<char*>(&CRC), 2);
	//std::stringstream os;
	if (CRC == checkCRC) {
		std::cout << "Header CRC ok\n";
	}
	else {
		exit(1);
	}

	unordered_map<uint8_t, definitionMessage> defList;

	//2) leggi il primo record
	bool outTime = true;
	for (size_t i = 0; i < header.data_size; ++i) {
		uint8_t def_header;
		is.read(reinterpret_cast<char*>(&def_header), 1);
		uint8_t type;
		uint8_t id;
		type = (def_header & 0b11110000) >> 4;
		id = def_header & 0b00001111;
		if (type == 4) { //message definition
			definitionMessage msgDef;
			// read message definition
			is.read(reinterpret_cast<char*>(&msgDef.reserved), 1);
			is.read(reinterpret_cast<char*>(&msgDef.arch), 1);
			is.read(reinterpret_cast<char*>(&msgDef.GlobMessageNumber), 2);
			is.read(reinterpret_cast<char*>(&msgDef.numFields), 1);
			i += 5;
			for (size_t j = 0; j < msgDef.numFields; ++j) {
				field f;
				is.read(reinterpret_cast<char*>(&f.number), 1);
				is.read(reinterpret_cast<char*>(&f.size), 1);
				is.read(reinterpret_cast<char*>(&f.base_type), 1);
				i += 3;
				msgDef.fieldDefinition.push_back(f);
			}
			defList[id] = msgDef;
		}
		else if (type == 0) { //message data
			definitionMessage msgDef = defList[id];
			vector<fieldData> data_vector;
			for (auto& fdef : msgDef.fieldDefinition) {
				if (fdef.size <= 8) {
					fieldData fd;
					fd.num = fdef.number;
					is.read(reinterpret_cast<char*>(&fd.value), fdef.size);
					i += fdef.size;
					data_vector.push_back(fd);
				}
				else {
					uint8_t flush;
					for (size_t k = 0; k < fdef.size; ++k) {
						is.read(reinterpret_cast<char*>(&flush), 1);
						++i;
					}
				}
				
			}
			//print time created
			if (outTime) {
				for (auto& f : data_vector) {
					if (f.num == 4) {
						std::cout << "time_created: " << f.value << '\n';
						outTime = !outTime;
					}
				}
			}
			
			// 3) report the speed for msg_type 19
			if (msgDef.GlobMessageNumber == 19) {
				for (auto& f : data_vector) {
					if (f.num == 13) {
						std::cout << "avg_speed: ";
						double speed = double(f.value) * 0.0036;
						std::cout << setprecision(4) << speed << '\n';
					}
				}
			}
		}
	}
	// 4) double-check CRC.
	uint32_t final_CRC = 0;
	checkCRC = 0;
	updateCrc(is, header.data_size, checkCRC);
	uint8_t a = is.peek();
	is.read(reinterpret_cast<char*>(&final_CRC), 2);
	if (final_CRC == checkCRC) {
		std::cout << "File CRC ok\n";
	}
	else {
		exit(1);
	}
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		error("Syntax: main <input_file.FIT>");
	}
	ifstream is(argv[1], ios::binary);
	if (!is) {
		error("Cannot open input file.");
	}

	readHeader(is);
}