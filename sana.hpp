#include<filesystem>
#include<fstream>
namespace fs = std::filesystem;
namespace sana{
	
	std::vector<char> ReadFile(const std::string& FileName) {
		std::ifstream File(FileName, std::ios::binary);
		std::vector<char> binaries(fs::exists(FileName) ? fs::file_size(FileName) : 0, '\0');
		if (File.is_open()) {
			File.read(binaries.data(), binaries.size());
		}
		File.close();
		return binaries;
	}
}