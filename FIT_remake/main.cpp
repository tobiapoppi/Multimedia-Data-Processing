#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <string>
#include <iomanip>


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

struct fit {

	struct header {
		uint8_t size_, v_;
		uint16_t pv_;
		uint32_t DRsize_;
		std::string mn_ = "    ";
		uint16_t crc_;
		uint16_t crc_check_ = 0;

		header(){}
		header(std::ifstream& is) {
			is.read(reinterpret_cast<char*>(&size_), 1);
			FitCRC_Get16(crc_check_, size_);

			is.read(reinterpret_cast<char*>(&v_), 1);
			FitCRC_Get16(crc_check_, v_);
			
			is.read(reinterpret_cast<char*>(&pv_), 2);
			FitCRC_Get16(crc_check_, pv_ & 0b0000000011111111);
			FitCRC_Get16(crc_check_, pv_ >> 8);
			
			is.read(reinterpret_cast<char*>(&DRsize_), 4);
			FitCRC_Get16(crc_check_, DRsize_ & 0x000000FF);
			FitCRC_Get16(crc_check_, (DRsize_ >> 8) & 0x000000FF);
			FitCRC_Get16(crc_check_, (DRsize_ >> 16) & 0x000000FF);
			FitCRC_Get16(crc_check_, DRsize_ >> 24);
			
			is.read(reinterpret_cast<char*>(&mn_[0]), 4);
			for (size_t i = 0; i < mn_.size(); ++i) {
				FitCRC_Get16(crc_check_, mn_[i]);
			}
			
			is.read(reinterpret_cast<char*>(&crc_), 2);
		}

		bool check_header_crc() {
			if (crc_check_ != crc_) {
				return false;
			}
			return true;
		}
	};

	std::ifstream& is_;
	header h_;


	fit(std::ifstream& is) : is_(is) {
		h_ = header(is_);
	}

	struct field {
		uint8_t num_, size_, basetype_;

		field(uint8_t n, uint8_t s, uint8_t bt) : num_(n), size_(s), basetype_(bt) {}
	};

	struct data_type {
		uint8_t num_, basetype_;
		uint64_t val_;

		data_type(uint8_t n, uint8_t bt, uint64_t v) : num_(n), basetype_(bt), val_(v) {}
	};

	void read_data_message(std::ifstream& is, uint8_t lmt) {
		std::vector<data_type> v;
		for (auto& x : definitions_[lmt]) {
			uint64_t val = 0;
			is.read(reinterpret_cast<char*>(&val), x.size_);
			this->h_.DRsize_ = this->h_.DRsize_ - x.size_;
			data_type d(x.num_, x.basetype_, val);
			v.push_back(d);
		}
		data_[lmt] = v;
	}

	void read_definition_message(std::ifstream& is, uint8_t lmt) {
		uint8_t rsv, arch, numf;
		uint16_t GMN;
		std::vector<field> fields;
		is.read(reinterpret_cast<char*>(&rsv), 1);
		FitCRC_Get16(this->h_.crc_check_, rsv);
		this->h_.DRsize_--;

		is.read(reinterpret_cast<char*>(&arch), 1);
		FitCRC_Get16(this->h_.crc_check_, arch);
		this->h_.DRsize_--;

		is.read(reinterpret_cast<char*>(&GMN), 2);
		FitCRC_Get16(this->h_.crc_check_, GMN & 0x00FF);
		FitCRC_Get16(this->h_.crc_check_, (GMN >> 8) & 0x00FF);
		this->h_.DRsize_--;
		this->h_.DRsize_--;

		is.read(reinterpret_cast<char*>(&numf), 1);
		FitCRC_Get16(this->h_.crc_check_, numf);
		this->h_.DRsize_--;

		for (size_t i = 0; i < numf; ++i) {
			uint8_t n, s, bt;
			is.read(reinterpret_cast<char*>(&n), 1);
			FitCRC_Get16(this->h_.crc_check_, n);
			this->h_.DRsize_--;

			is.read(reinterpret_cast<char*>(&s), 1);
			FitCRC_Get16(this->h_.crc_check_, s);
			this->h_.DRsize_--;

			is.read(reinterpret_cast<char*>(&bt), 1);
			FitCRC_Get16(this->h_.crc_check_, bt);
			this->h_.DRsize_--;
			
			fields.emplace_back(field(n, s, bt));
		}
		definitions_[lmt] = fields;
		glob_loc_[GMN] = lmt;

	};

	void read_data_record() {
		uint8_t dr_header;

		is_.read(reinterpret_cast<char*>(&dr_header), 1);
		FitCRC_Get16(this->h_.crc_check_, dr_header);
		this->h_.DRsize_--;

		uint8_t local_message_type = dr_header & 0x0F;

		if ((dr_header >> 4) == 0) {
			//data message
			read_data_message(is_, local_message_type);
		}
		else {
			//definition message
			read_definition_message(is_, local_message_type);
		}
	}

	std::map<uint8_t, std::vector<field>> definitions_;
	std::map<uint8_t, uint8_t> glob_loc_;
	std::map<uint8_t, std::vector<data_type>> data_;
};


bool read_fit(std::ifstream& is) {
	fit f(is);
	if (!f.h_.check_header_crc()) {
		return false;
	}
	std::cout << "Header CRC ok";
	while (int32_t(f.h_.DRsize_) > 0) {
		f.read_data_record();
	}

	const auto& loc = f.glob_loc_.find(19);
	const auto& v = f.data_.find(loc->first);
	for (const auto& d : v->second) {
		if (d.num_ == 13) {
			std::cout << "avg_speed = " << d.val_ / 1000000 * 3600 << std::setprecision(3) << " km/h";
		}
	}

}

int main(int argc, char **argv)
{
	std::ifstream is(argv[1], std::ios::binary);

	if (!read_fit(is)) {
		return(EXIT_FAILURE);
	}
	return(EXIT_SUCCESS);

}