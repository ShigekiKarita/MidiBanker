#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <optional>

/**
	Parser state with current stream and previous line. 
*/
struct ParserState
{
	std::ifstream* stream = nullptr;
	std::string prev_line = "";

	std::optional<std::string> next()
	{
		std::string ret;
		if (!this->prev_line.empty())
		{
			auto ret = prev_line;
			this->prev_line = "";
			return ret;
		}
		else if (std::getline(*stream, ret))
		{
			return ret;
		}
		return std::nullopt;
	}
};

/**
	 The program names are associated with MIDI messages Program Change and Bank select MSB and LSB.

	 [p%L,%n,%n]%s
	 define program name:
	 %L level [1 - 9], %n [0 - 127] program change
	 %n bank (14 bit) [0 - 16383], %s program name
*/
struct Program
{
	char level;
	char program_change;
	int bank;
	std::string name;
};

struct Group
{
	int level;
	std::string name;
	std::vector<Program> patches;
	std::vector<Group> subgroups;
};

struct Mode
{
	std::string name;
	std::vector<Group> groups;
};

std::ostream& operator<<(std::ostream& os, const Mode& mode)
{
	return os << "Mode(\"" << mode.name << "\")";
}

struct Device
{
	std::string manufacturer;
	std::string name;
	std::string script_name;

	std::vector<Mode> modes;

	void load_file(ParserState& state);
};

bool starts_with(const std::string& full, const std::string& query)
{
	if (full.size() < query.size()) return false;
	return std::equal(full.begin(), full.begin() + query.size(), query.begin());
}

struct Line
{
	std::string key;
	std::string value;
};

// line should be "[key]value"
std::optional<Line> load_line(const std::string& line)
{
	// extract key
	auto k_begin = line.find("[");
	auto k_end = line.find("]");
	if (k_end == std::string::npos || k_begin == std::string::npos)
	{
		return std::nullopt;
	}
	auto key = line.substr(k_begin + 1, k_end - k_begin - 1);

	// extract value
	auto rest = line.substr(k_end + 1, line.size() - key.size() - 2);
	std::istringstream iss(rest);
	std::ostringstream oss;
	while (iss)
	{
		std::string tmp;
		iss >> tmp;
		oss << tmp << " ";
	}
	auto value = oss.str();
	if (value.size() > 1)
	{
		value.resize(value.size() - 2);
	}
	Line ret = {key, value};
	return ret;
}


void Device::load_file(ParserState& state)
{
	while (auto line_str = state.next())
	{
		auto opt_line = load_line(line_str.value());
		if (!opt_line) continue;

		auto line = opt_line.value();
		if (line.key.empty()) continue;
		if (line.key == "device manufacturer")
		{
			this->manufacturer = line.value;
		}
		else if (line.key == "device name")
		{
			this->name = line.value;
		}
		else if (line.key == "script name")
		{
			this->script_name = line.value;
		}
		else if (starts_with(line.key, "mode"))
		{
			Mode mode;
			mode.name = line.value;
			// mode.load_file(state);
			this->modes.push_back(mode);
		}
		else
		{
			// std::cout << "key: " << line.key << std::endl;
			// std::cout << "value: " << line.value << std::endl;
		}
	}
}


std::ostream& operator<<(std::ostream& os, const Device& dev)
{
	os << "Device(\n"
		 << " name: " << dev.name << "\n"
		 << " script name: " << dev.script_name << "\n"
		 << " manufacturer: " << dev.manufacturer << "\n"
		 << " modes={";

	for (auto&& m : dev.modes)
	{
		os << m << ",";
	}
	os << "}\n";
	os << ")";
	return os;
}

void test_parser()
{
	{
		assert(starts_with("hogefuga", "hoge"));
	}
	{
		auto line = load_line("[key]value").value();
		assert(line.key == "key");
		assert(line.value == "value");
	}
	{
		auto line = load_line("[key bar ]value foo").value();
		assert(line.key == "key bar ");
		assert(line.value == "value foo");
	}
}

int main(int argc, char** argv)
{
	test_parser();
	
	if (argc != 2)
	{
		std::cerr << "usage: $ " << argv[0] << " patch.txt" << std::endl;
		return 1;
	}
	std::string filename = argv[1];
	std::ifstream ifs(filename);
	if (!ifs)
	{
		std::cerr << filename << " not found!" << std::endl;
		return 1;
	}

	ParserState state {&ifs};

	Device dev;
	dev.load_file(state);
	std::cout << dev << std::endl;	
	return 0;
}
