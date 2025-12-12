#include <filesystem>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>

#define DEFAULT_PATH "emmgbuild.txt";
#define MAKE_PATH "Makefile";

template <typename T>
int removeVectorElement(std::vector<T>& vector, T element) {
	auto it = std::remove(vector.begin(), vector.end(), element);
	if(it != vector.end())  {
		vector.erase(it, vector.end());
		return 0;
	}
	return 1;
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

#define COMPILER "${CXX}";
#define LINKER "${CXX}";

#define SRC "src";
#define OBJ "obj"
#define BUILD "build"

int main(int argc, char** argv) {
	const std::string helpText = 
	"Easy macro makefile generator"
	"Usage: emmg [options]" "\n"
	"Will read emmgbuild.txt by default." "\n"
	"Options:" "\n"
	"\t-w: Compile Makefile for Windows." "\n"
	"\t-l: Compile Makefile for Linux (default)." "\n"
	"\t-f[filepath]: Use filepath instead of emmgbuild.txt (no spaces)." "\n"
	"\t-o[filepath]: Use filepath instead of writing to Makefile (no spaces)." "\n"
	"\t-g: Generate template emmgbuild.txt. Will ignore all other flags other than -f to change where template is generated." "\n"
	;

	std::string readPath = DEFAULT_PATH;
	std::string outPath = MAKE_PATH;

	char mode = 'l';
	bool genMake = false;
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
		if(argStr.rfind("-o",0) == 0) {
			outPath = argStr.substr(2);
		} else
		if(argStr == "-h") {
			std::cout << helpText << std::endl;
			return 0;
		} else
		if(argStr == "-g") {
			genMake = true;
		}
		else {
			std::cout << "Unknown option." << std::endl;
			return 1;
		}
	}
	if(genMake) {
		if(std::filesystem::exists(readPath)) {
			std::cout << "The path for the template already exists. Continue? y/N\n";
			char input;
			std::cin >> input;
			if(!(input == 'y' || input == 'Y')) return 0;
		}
		std::ofstream templateOut(readPath);
		std::string templateString =
			"dir obj" "\n"
			"dir build" "\n"
			"" "\n"
			"# Put src here" "\n"
			"" "\n"
			"# Put build here" "\n"
			"" "\n"
			"embedmake" "\n"
			"clean:" "\n"
			"	rm -r obj" "\n"
			"	rm -r build" "\n"
			".PHONY: clean" "\n"
			"endmake"
			;
		templateOut << templateString;
		return 0;
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

	std::vector<std::string> srcFlagsL;
	std::vector<std::string> buildFlagsL;

	std::vector<std::string> srcFlagsW;
	std::vector<std::string> buildFlagsW;

	std::vector<std::string> extrachecks;

	int cmdCounter = 0;
	std::string commandGen = "";

	bool useAll = true;

	makefileOut += (std::string)"OSMODE := " + mode + "\n\n";

	int lineNumber = 0;
	int inMakeEmbed = 0;
	while(std::getline(fileStream, line)) {
		lineNumber++;
		std::vector<std::string> parts = split(line);
		if(inMakeEmbed > 0) {
			if(parts.at(0) == "endmake") {
				if(inMakeEmbed > 1) {
					makefileOut += "endif\n";
				}
				makefileOut += "\n";
				inMakeEmbed = false;
				continue;
			}
			makefileOut += line + "\n";
			continue;
		} else
		if(parts.size() < 1) {
			continue; 
		} else
		if(parts.at(0) == "#") {
			continue; 
		} else
		if(parts.at(0).at(0) == '#') {
			// ^ funny
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
			std::string extra = "";
			for(auto check : extrachecks) {
				extra+=check+" ";
			}

			makefileOut += parts.at(1) + "/: " + extra + "\n";
			makefileOut += "ifeq (${OSMODE}, l)\n";
			makefileOut += "\tmkdir -p " + parts.at(1);
			makefileOut += "\nelse\n";
			makefileOut += "\tmkdir " + parts.at(1);
			makefileOut += "\nendif";
			makefileOut += "\n\n";

			if(useAll)
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
			std::string flagsL = "";
			for(auto i : srcFlagsL) flagsL+= " " + i;

			std::string flagsW = "";
			for(auto i : srcFlagsW) flagsW+= " " + i;

			std::string extra = "";
			for(auto check : extrachecks) {
				extra+=check+" ";
			}

			std::string objFilepath = objPath + "/" + parts.at(1) + ".o";
			makefileOut += objFilepath +": " + srcPath + "/" + parts.at(1) + " " + extra + "\n";
			makefileOut += "ifeq (${OSMODE}, l)\n";
			makefileOut += "\t" + compiler + " " + srcPath + "/" + parts.at(1) + " -c -o " + objFilepath + flagsL;
			makefileOut += "\nelse\n";
			makefileOut += "\t" + compiler + " " + srcPath + "/" + parts.at(1) + " -c -o " + objFilepath + flagsW;
			makefileOut += "\nendif\n";
			makefileOut += "\n\n";

			commandGen += "ifeq (${OSMODE}, l)\n";
			commandGen += "\t" + (std::string)"clang" + " " + srcPath + "/" + parts.at(1) + " " + flagsL + " -MJ emmgtemp/" + std::to_string(cmdCounter) + ".json -fsyntax-only";
			commandGen += "\nelse\n";
			commandGen += "\t" + (std::string)"clang" + " " + srcPath + "/" + parts.at(1) + " " + flagsW + " -MJ emmgtemp/" + std::to_string(cmdCounter) + ".json -fsyntax-only";
			commandGen += "\nendif\n";
			cmdCounter++;

			if(useAll)
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
			if(buildTargets.empty()) {
				std::cerr << "No targets for operation \"build\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string flagsL = "";
			for(auto i : buildFlagsL) flagsL+= " " + i;

			std::string flagsW = "";
			for(auto i : buildFlagsW) flagsW+= " " + i;

			makefileOut += buildPath + "/" + parts.at(1) + ": ";
			std::string rules = "";
			for(auto rule : buildTargets) {
				rules += rule + " ";
			}

			std::string extra = "";
			for(auto check : extrachecks) {
				extra+=check+" ";
			}


			makefileOut += rules;
			// rules will already have a space at the end and itl look kinda ugly if add this space lowk
			//makefileOut += " ";
			makefileOut += extra;
			makefileOut += "\n";
			makefileOut += "ifeq (${OSMODE}, l)\n";
			makefileOut += "\t" + linker + " " + rules + "-o " + buildPath + "/" + parts.at(1) + flagsL;
			makefileOut += "\nelse\n";
			makefileOut += "\t" + linker + " " + rules + "-o " + buildPath + "/" + parts.at(1) + flagsW;
			makefileOut += "\nendif\n";
			makefileOut += "\n\n";
			if(useAll)
				all.push_back(buildPath + "/" + parts.at(1));
		} else if(parts.at(0) == "srcdir") {
			if(parts.size()>2) {
				std::cerr << "Too many directories for operation \"srcdir\" at line " << lineNumber << std::endl;
				return 1;
			}
			if(parts.size()<2) {
				srcPath = SRC;
			} else {
				srcPath = parts.at(1);
			}
		} else if(parts.at(0) == "objdir") {
			if(parts.size()>2) {
				std::cerr << "Too many directories for operation \"objdir\" at line " << lineNumber << std::endl;
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
		} else if(parts.at(0) == "srcflagL") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"srcflag\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string after = "";
			for(int i = 1; i<parts.size(); i++) {
				after += parts.at(i) + " ";
			}
			after.pop_back();
			srcFlagsL.push_back(after);
		} else if(parts.at(0) == "rmsrcflagL") {
			if(parts.size()<2) {
				srcFlagsL.clear();
			} else {
				std::string after = "";
				for(int i = 1; i<parts.size(); i++) {
					after += parts.at(i) + " ";
				}
				after.pop_back();
				if(removeVectorElement(srcFlagsL, after)) {
					std::cerr << "Warning: trying to remove nonexistent flag \"" << after << "\" on line " << lineNumber << "\n";
				}
			}
		} else if(parts.at(0) == "buildflagL") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"objflag\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string after = "";
			for(int i = 1; i<parts.size(); i++) {
				after += parts.at(i) + " ";
			}
			after.pop_back();
			buildFlagsL.push_back(after);
		} else if(parts.at(0) == "rmbuildflagL") {
			if(parts.size()<2) {
				buildFlagsL.clear();
			} else {
				std::string after = "";
				for(int i = 1; i<parts.size(); i++) {
					after += parts.at(i) + " ";
				}
				after.pop_back();
				if(removeVectorElement(buildFlagsL, after)) {
					std::cerr << "Warning: trying to remove nonexistent flag \"" << after << "\" on line " << lineNumber << "\n";
				}
			}
		} else if(parts.at(0) == "srcflagW") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"srcflag\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string after = "";
			for(int i = 1; i<parts.size(); i++) {
				after += parts.at(i) + " ";
			}
			after.pop_back();
			srcFlagsW.push_back(after);
		} else if(parts.at(0) == "rmsrcflagW") {
			if(parts.size()<2) {
				srcFlagsW.clear();
			} else {
				std::string after = "";
				for(int i = 1; i<parts.size(); i++) {
					after += parts.at(i) + " ";
				}
				after.pop_back();
				if(removeVectorElement(srcFlagsW, after)) {
					std::cerr << "Warning: trying to remove nonexistent flag \"" << after << "\" on line " << lineNumber << "\n";
				}
			}
		} else if(parts.at(0) == "buildflagW") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"objflag\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string after = "";
			for(int i = 1; i<parts.size(); i++) {
				after += parts.at(i) + " ";
			}
			after.pop_back();
			buildFlagsW.push_back(after);
		} else if(parts.at(0) == "rmbuildflagW") {
			if(parts.size()<2) {
				buildFlagsW.clear();
			} else {
				std::string after = "";
				for(int i = 1; i<parts.size(); i++) {
					after += parts.at(i) + " ";
				}
				after.pop_back();
				if(removeVectorElement(buildFlagsW, after)) {
					std::cerr << "Warning: trying to remove nonexistent flag \"" << after << "\" on line " << lineNumber << "\n";
				}
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
			srcFlagsW.push_back(after);
			srcFlagsL.push_back(after);
		} else if(parts.at(0) == "rmsrcflag") {
			if(parts.size()<2) {
				srcFlagsW.clear();
			} else {
				std::string after = "";
				for(int i = 1; i<parts.size(); i++) {
					after += parts.at(i) + " ";
				}
				after.pop_back();
				if(removeVectorElement(srcFlagsW, after)) {
					std::cerr << "Warning: trying to remove nonexistent Windows flag \"" << after << "\" on line " << lineNumber << "\n";
					return 1;
				}
				if(removeVectorElement(srcFlagsL, after)) {
					std::cerr << "Warning: trying to remove nonexistent Linux flag \"" << after << "\" on line " << lineNumber << "\n";
					return 1;
				}
			}
		} else if(parts.at(0) == "buildflag") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"objflag\" at line " << lineNumber << std::endl;
				return 1;
			}
			std::string after = "";
			for(int i = 1; i<parts.size(); i++) {
				after += parts.at(i) + " ";
			}
			after.pop_back();
			buildFlagsW.push_back(after);
			buildFlagsL.push_back(after);
		} else if(parts.at(0) == "rmbuildflag") {
			if(parts.size()<2) {
				buildFlagsW.clear();
			} else {
				std::string after = "";
				for(int i = 1; i<parts.size(); i++) {
					after += parts.at(i) + " ";
				}
				after.pop_back();
				if(removeVectorElement(buildFlagsW, after)) {
					std::cerr << "Warning: trying to remove nonexistent Windows flag \"" << after << "\" on line " << lineNumber << "\n";
					return 1;
				}
				if(removeVectorElement(buildFlagsL, after)) {
					std::cerr << "Warning: trying to remove nonexistent Linux flag \"" << after << "\" on line " << lineNumber << "\n";
					return 1;
				}
			}
		} else if(parts.at(0) == "all") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"all\" at line " << lineNumber << std::endl;
				return 1;
			}
			for(int i = 1; i<parts.size();i++) {
				all.push_back(parts.at(i));
			}
		} else if(parts.at(0) == "stopall") {
			useAll = false;
		} else if(parts.at(0) == "startall") {
			useAll = true;
		} else if(parts.at(0) == "addbuild") {
			if(parts.size()<2) {
				std::cerr << "No argument for operation \"addbuild\" at line " << lineNumber << std::endl;
				return 1;
			}
			for(int i = 1; i<parts.size();i++) {
				buildTargets.push_back(parts.at(i));
			}

		} else if(parts.at(0) == "rmbuild") {
			if(parts.size()<2) {
				buildTargets.clear();
			} else {
				for(int i = 1; i<parts.size(); i++) {
					if(removeVectorElement(buildTargets, parts.at(i))) {
						std::cerr << "Warning: trying to remove nonexistent target \"" << parts.at(i) << "\" on line " << lineNumber << "\n";
						return 1;
					}
				}
			}
		} else if(parts.at(0) == "embedmake") {
			if(parts.size()>1) {
				std::cerr << "Embedding make does not take any arguments. Line " << lineNumber << std::endl;
				return 1;
			}
			inMakeEmbed = 1;
		} else if(parts.at(0) == "embedmakeL") {
			if(parts.size()>1) {
				std::cerr << "Embedding make does not take any arguments. Line " << lineNumber << std::endl;
				return 1;
			}
			inMakeEmbed = 2;
			makefileOut += "ifeq (${OSMODE}, l)\n";
		} else if(parts.at(0) == "embedmakeW") {
			if(parts.size()>1) {
				std::cerr << "Embedding make does not take any arguments. Line " << lineNumber << std::endl;
				return 1;
			}
			inMakeEmbed = 2;
			makefileOut += "ifeq (${OSMODE}, w)\n";
		} else if(parts.at(0) == "check") {
			if(parts.size()<2) {
				std::cerr << "No argmuent for operation \"check\" at line " << lineNumber << std::endl;
				return 1;
			}
			for(int i = 1; i<parts.size(); i++) {
				extrachecks.push_back(parts.at(i));
			}
		} else if(parts.at(0) == "rmcheck") {
			if(parts.size()<2) {
				extrachecks.clear();
			} else {
				for(int i = 1; i<parts.size(); i++) {
					if(removeVectorElement(extrachecks, parts.at(i))) {
						std::cerr << "Warning: trying to remove nonexistent check \"" << parts.at(i) << "\" on line " << lineNumber << "\n";
						return 1;
					}
				}
			}
		} else if(parts.at(0) == "compiler") {
			if(parts.size()<2) {
				compiler = COMPILER;
			} else {
				std::string temp = "";
				
				for(int i = 1; i<parts.size(); i++) {
					temp+=parts.at(i) + " ";
					
				}
				temp.pop_back();

				compiler = temp;
			}
		} else if(parts.at(0) == "linker") {
			if(parts.size()<2) {
				linker = LINKER;
			} else {
				std::string temp = "";

				for(int i = 1; i<parts.size(); i++) {
					temp+=parts.at(i) + " ";
				}
				temp.pop_back();

				linker = temp;
			}
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

	makefileOut += "\ngencommands:\n\tmkdir emmgtemp\n";
	makefileOut += commandGen;
	makefileOut += "# not cross platform here sad i think\n";
	makefileOut += "\techo [ > emmgtemp/[\n";
	makefileOut += "\techo ] > emmgtemp/]\n";
	makefileOut += "\tcat emmgtemp/[ emmgtemp/*.json emmgtemp/] > compile_commands.json\n";
	makefileOut += "\trm -r emmgtemp\n";
	
	//std::cout << makefileOut;
	std::ofstream fileOutStream(outPath);
	fileOutStream << makefileOut;
}
