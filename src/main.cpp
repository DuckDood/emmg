#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>

#define DEFAULT_PATH "emmgbuild.txt";

template <typename T>
void removeVectorElement(std::vector<T>& vector, T element) {
	vector.erase(std::remove(vector.begin(), vector.end(), element), vector.end());
}

std::string readFile(std::string path) {
	std::ifstream file(path);
	if(!file.good()) {
		return "";
	}
	std::string out = "";
	// i know it adds an extra newine at the end but whatever its not gonna matter here
	for(std::string line; std::getline(file, line); out+=line+"\n");
	return out;
}

std::vector<std::string> split(std::string line, char delim = ' ') {
	std::istringstream lineStream(line);
	std::string element;
	std::vector<std::string> out;
	while(std::getline(lineStream, element, delim)) {
		out.push_back(element);
	}
	return out;
}

#define COMPILER "g++";
#define LINKER "g++";

#define SRC "src";
#define OBJ "obj"
#define BUILD "build"

int main(int argc, char** argv) {
	const std::string helpText = 
	"Usage: emmg [options]" "\n"
	"Will read emmgbuild.txt by default." "\n"
	"Options:" "\n"
	"\t-w: Compile Makefile for Windows." "\n"
	"\t-l: Compile Makefile for Linux (default)." "\n"
	"\t-f[filepath]: Use filepath (no spaces)." "\n"
	;

	std::string readPath = DEFAULT_PATH;

	char mode = 'l';
	for(int i = 1; i<argc; i++) {
		std::string argStr = argv[i];
		if(argStr == "-w") {
			mode = 'w';
		} else
		if(argStr == "-l") {
			mode = 'l';
		} else
		if(argStr.rfind("-f",0) == 0) {
			readPath = argStr.substr(2);
		} else
		if(argStr == "-h") {
			std::cout << helpText << std::endl;
			return 0;
		} else {
			std::cout << "Unknown option." << std::endl;
			return 1;
		}
	}

	std::string file = readFile(readPath);
	if(file.empty()) {
		std::cout << "File is blank or does not exist." << std::endl;
	}
	std::string makefileOut = "";
	std::string line;
	std::istringstream fileStream(file);
	std::string compiler = COMPILER;
	std::string linker = LINKER;

	std::string srcPath = SRC;
	std::string objPath = OBJ;
	std::string buildPath = BUILD;
	std::vector<std::string> all;

	std::vector<std::string> buildTargets;

	std::vector<std::string> srcFlags;
	std::vector<std::string> buildFlags;

	makefileOut += (std::string)"OSMODE := " + mode + "\n\n";

	int lineNumber = 0;
	while(std::getline(fileStream, line)) {
		lineNumber++;
		std::vector<std::string> parts = split(line);
		if(parts.size() < 1) {
			continue; 
		} else
		if(parts.at(0) == "#") {
			continue; 
		} else
		if(parts.at(0) == "dir") {
			if(parts.size()<2) {
				std::cerr << "No directory name for operation \"dir\" at line " << lineNumber << std::endl;
				return 1;
			}
			if(parts.size()>2) {
				std::cerr << "Too many directory names for operation \"dir\" at line " << lineNumber << std::endl;
				return 1;
			}

			makefileOut += parts.at(1) + "/:\n";
			makefileOut += "ifeq (${OSMODE}, l)\n";
			makefileOut += "\tmkdir -p " + parts.at(1);
			makefileOut += "\nelse\n";
			makefileOut += "\tmkdir " + parts.at(1);
			makefileOut += "\nendif";
			makefileOut += "\n\n";

			all.push_back(parts.at(1) + "/");
		} else if(parts.at(0) == "src") {
			if(parts.size()<2) {
				std::cerr << "No source filename for operation \"src\" at line " << lineNumber << std::endl;
				return 1;
			}
			if(parts.size()>2) {
				std::cerr << "Too many source filenames for operation \"src\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string flags = "";
			for(auto i : srcFlags) flags+= " " + i;
			std::string objFilepath = objPath + "/" + parts.at(1) + ".o";
			makefileOut += objFilepath +": " + srcPath + "/" + parts.at(1) + "\n";
			makefileOut += "\t" + compiler + " " + srcPath + "/" + parts.at(1) + " -c -o " + objFilepath + flags;
			makefileOut += "\n\n";
			all.push_back(objFilepath);

			buildTargets.push_back(objFilepath);
		} else if(parts.at(0) == "build") {
			if(parts.size()<2) {
				std::cerr << "No output filename for operation \"build\" at line " << lineNumber << std::endl;
				return 1;
			}
			if(parts.size()>2) {
				std::cerr << "Too many output filenames for operation \"build\" at line " << lineNumber << std::endl;
				return 1;
			}
			makefileOut += buildPath + "/" + parts.at(1) + ": ";
			std::string rules = "";
			for(auto rule : buildTargets) {
				rules += rule + " ";
			}
			makefileOut += rules;
			makefileOut += "\n";
			makefileOut += "\t" + linker + " " + rules + "-o " + buildPath + "/" + parts.at(1);
			makefileOut += "\n\n";
			all.push_back(buildPath + "/" + parts.at(1));
		} else if(parts.at(0) == "srcdir") {
			if(parts.size()>2) {
				std::cerr << "Too directories for operation \"srcdir\" at line " << lineNumber << std::endl;
				return 1;
			}
			if(parts.size()<2) {
				srcPath = SRC;
			} else {
				srcPath = parts.at(1);
			}
		} else if(parts.at(0) == "objdir") {
			if(parts.size()>2) {
				std::cerr << "Too directories for operation \"objdir\" at line " << lineNumber << std::endl;
				return 1;
			}
			if(parts.size()<2) {
				objPath = OBJ;
			} else {
				objPath = parts.at(1);
			}
		} else if(parts.at(0) == "builddir") {
			if(parts.size()>2) {
				std::cerr << "Too many directories for operation \"builddir\" at line " << lineNumber << std::endl;
				return 1;
			}
			if(parts.size()<2) {
				buildPath = BUILD;
			} else {
				buildPath = parts.at(1);
			}
		} else if(parts.at(0) == "srcflag") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"srcflag\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string after = "";
			for(int i = 1; i<parts.size(); i++) {
				after += parts.at(i) + " ";
			}
			after.pop_back();
			srcFlags.push_back(after);
		} else if(parts.at(0) == "rmsrcflag") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"rmsrcflag\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string after = "";
			for(int i = 1; i<parts.size(); i++) {
				after += parts.at(i) + " ";
			}
			after.pop_back();
			removeVectorElement(srcFlags, after);
		}

		else {
			std::cerr << "Unknown operation at line " << lineNumber << std::endl;
			return 1;
		}
	}
	std::string allStr = "all: ";
	for(auto target : all) {
		allStr += target + " ";
	}
	makefileOut = allStr + "\n" + makefileOut;
	
	std::cout << makefileOut;
}
