#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <map>
#include <io.h>
#include <Windows.h>


const bool compression = false;

struct Mesh{std::map<std::string,std::vector<std::string>> Builds;};
struct BuildDefinition{std::string Comparer;std::string Command;std::string Output;};
struct content{std::string dir;std::string name;bool follow;BuildDefinition filter;};

std::map<std::string,BuildDefinition> buildDefinitions;
std::vector<content> dirsToLookAt;
std::map<std::string,Mesh> foundItems;
std::map<std::string,Mesh> toCfg;
std::map<std::string,std::vector<std::string>> additionalCommands;

std::string sol_dir;
std::string out_dir;

void scanDir(content c);
void build(const std::string attrib,std::vector<std::string> data, const std::string cmdline);
void addToCfg(const std::string attrib, const std::string file);
unsigned long command(std::string cmdline, std::string path);
void writeMeshesToCfg(std::string fname);

int main(int argc, char** argv)
{
	std::ifstream stream(argv[1]);
	sol_dir = argv[2];
	out_dir = argv[3];
	
	buildDefinitions["mesh"].Comparer = ".obj";
	buildDefinitions["mesh"].Command = sol_dir + "../../external/mesh_tools/obj2t3d.exe -i \"%d\" -o \"" + out_dir + "%f.t3d\" -y";
	buildDefinitions["mesh"].Output = "%f.t3d";

	buildDefinitions["diffuse"].Comparer = ".png,diffuse";
	buildDefinitions["diffuse"].Command = sol_dir + "../../external/terrain_tools/ged-texture-converter/nBuildDrv.exe -verbose -2 -p \"Input=%d\" -p \"Output=" + out_dir + "%f.ntx\" -p \"Input-Type=Color\" -p \"Compress="+(compression?"true":"false")+"\" \"BuildTools.TextureImporter\"";
	buildDefinitions["diffuse"].Output = "%f.ntx";

	buildDefinitions["specular"].Comparer = ".png,spec";
	buildDefinitions["specular"].Command = sol_dir + "../../external/terrain_tools/ged-texture-converter/nBuildDrv.exe -verbose -2 -p \"Input=%d\" -p \"Output=" + out_dir + "%f.ntx\" -p \"Input-Type=Color\" -p \"Compress="+(compression?"true":"false")+"\" \"BuildTools.TextureImporter\"";
	buildDefinitions["specular"].Output = "%f.ntx";

	buildDefinitions["glow"].Comparer = ".png,glow";
	buildDefinitions["glow"].Command = sol_dir + "../../external/terrain_tools/ged-texture-converter/nBuildDrv.exe -verbose -2 -p \"Input=%d\" -p \"Output=" + out_dir + "%f.ntx\" -p \"Input-Type=Color\" -p \"Compress="+(compression?"true":"false")+"\" \"BuildTools.TextureImporter\"";
	buildDefinitions["glow"].Output = "%f.ntx";

	buildDefinitions["normal"].Comparer = ".png,normal";
	buildDefinitions["normal"].Command = sol_dir + "../../external/terrain_tools/ged-texture-converter/nBuildDrv.exe -verbose -2 -p \"Input=%d\" -p \"Output=" + out_dir + "%f.ntx\" -p \"Input-Type=XY-Normal\" -p \"Compress="+(compression?"true":"false")+"\" \"BuildTools.TextureImporter\"";
	buildDefinitions["normal"].Output = "%f.ntx";

	while(!stream.eof())
	{
		std::string var;
		stream >> var;

		if(var.compare("Directory")==0)
		{
			content dir;
			stream >> dir.name >> dir.dir;
			dir.dir = sol_dir + "..\\..\\external\\art\\" + dir.dir; 
			dir.follow = true;
			dirsToLookAt.push_back(dir);
		}
		if(var.compare("Directory-NoFollow")==0)
		{
			content dir;
			stream >> dir.name >> dir.dir;
			dir.dir = sol_dir + "..\\..\\external\\art\\" + dir.dir; 
			dir.follow = false;
			dirsToLookAt.push_back(dir);
		}
		if(var.compare("Command")==0)
		{
			std::string s;
			std::getline(stream,s,'\n');
			s = s.substr(1);
			additionalCommands[s].push_back("");
		}
		if(var.compare("Directory-Filter")==0)
		{
			content dir;
			stream>>dir.name>>dir.dir>>dir.filter.Comparer>>dir.filter.Command;
			dir.filter.Command = (buildDefinitions.find(dir.filter.Command)!=buildDefinitions.end())?buildDefinitions[dir.filter.Command].Command:dir.filter.Command;
			dir.filter.Output = "%f";
			dir.dir = sol_dir + "..\\..\\external\\art\\" + dir.dir; 
			dir.follow = true;
			dirsToLookAt.push_back(dir);
		}
	}

	stream.close();

	for each(content c in dirsToLookAt)
	{
		scanDir(c);
	}

	for(auto i=foundItems.begin();i!=foundItems.end();i++)
	{
		for(auto j=i->second.Builds.begin();j!=i->second.Builds.end();j++)
		{
			build(i->first+"/"+j->first,j->second,buildDefinitions[j->first].Command);
		}
	}

	for each(std::pair<std::string,std::vector<std::string>> s in additionalCommands)
	{
		for each(std::string d in s.second)
		{
			command(s.first,d);
		}
	}

	if(argc>=5)
	{
		writeMeshesToCfg(argv[4]);
	}

	return 0;
}

void scanDir(content c)
{
		std::string cur = c.dir + "\\*";
		long hFile;
		struct _finddata_t c_file;

		if( (hFile = _findfirst( cur.c_str(), &c_file )) == -1L )
		{
			fprintf(stderr, "Error opening %s!\n", cur.c_str());
			return;
		}

		do
		{
			//skip if find . and ..
			if ((strcmp(c_file.name, ".") == 0 ||  strcmp(c_file.name, "..") == 0)) {
			     continue;
			}

			if (c_file.attrib & _A_SUBDIR){
				if(c.follow)
				{
					content d = c;
					d.dir = std::string(c.dir) + "\\" + c_file.name;
					scanDir(d);
				}
				continue;
			}

			char ncpy[sizeof(c_file.name)];
			strcpy_s(ncpy,c_file.name);
			_strlwr_s(ncpy);
			std::string fname = ncpy;

			if(c.filter.Comparer.length()==0)
			{
				for(auto i=buildDefinitions.begin();i!=buildDefinitions.end();i++)
				{
					std::stringstream ss(i->second.Comparer);
					std::string result;
					bool found = true;
					while(std::getline(ss,result,','))
					{
						if(fname.find(result) == fname.npos)
						{
							found = false;
							break;
						}
					}
					if(found) 
					{
						foundItems[c.name].Builds[i->first].push_back(std::string(c.dir) + "\\" + c_file.name);
						break;
					}
				}
			}
			else
			{
				std::stringstream ss(c.filter.Comparer);
				std::string result;
				bool found = true;
				while(std::getline(ss,result,','))
				{
					if(fname.find(result) == fname.npos)
					{
						found = false;
						break;
					}
				}
				if(found) 
				{
					additionalCommands[c.filter.Command].push_back(std::string(c.dir) + "\\" + c_file.name);
				}
			}
		}
		while(_findnext( hFile, &c_file ) == 0);
}

void build(const std::string attrib,std::vector<std::string> data,const std::string cmdline)
{
	if(data.size()>1)
		{
			std::cout<<std::endl<<"Multiple entries found for "<<attrib<<"!"<<std::endl;
			std::cout<<"Select options (comma separated):"<<std::endl<<"0: Build all"<<std::endl;
			int j=1;
			for each(std::string s in data)
			{
				std::cout<<(j++)<<": Build "<<s.substr(s.find_last_of('\\')+1,s.find_last_of('.')-s.find_last_of('\\')-1)<<std::endl;
			}
			std::cout<<"Input: ";
			std::string input;
			std::cin>>input;
			std::cout<<std::endl;
			if(input.compare("0")==0||input.length()==0)
			{
				for each(std::string s in data)
				{
					if(command(cmdline,s)==0) addToCfg(attrib,s.substr(s.find_last_of('\\')+1,s.find_last_of('.')-s.find_last_of('\\')-1));
				}
			}
			else
			{
				std::vector<std::string> results;
				std::stringstream ss(input);
				std::string item;
				while(std::getline(ss, item, ',')) {
					results.push_back(item);
				}

				for each(std::string u in results)
				{
					unsigned int t = atoi(u.c_str());
					if(t<=data.size()&&t!=0)
					{
						if(command(cmdline,data[t-1])==0) addToCfg(attrib,data[t-1].substr(data[t-1].find_last_of('\\')+1,data[t-1].find_last_of('.')-data[t-1].find_last_of('\\')-1));
					}
				}
			}
		}
	else if(data.size()!=0)
	{
		if(command(cmdline,data[0])==0) addToCfg(attrib,data[0].substr(data[0].find_last_of('\\')+1,data[0].find_last_of('.')-data[0].find_last_of('\\')-1));
	}
}


void addToCfg(const std::string attrib, const std::string file)
{
	std::stringstream a(attrib);
	std::string name;
	std::string type;
	std::getline(a,name,'/');
	std::getline(a,type,'/');

	Mesh m;
	if(toCfg.find(name)==toCfg.end()) toCfg[name] = m;

	std::string outName = std::string(buildDefinitions[type].Output);
	outName.replace(outName.find("%f"),2,file);

	toCfg[name].Builds[type].push_back(outName);
}

unsigned long command(std::string cmdline, std::string path)
{
	std::string cmd = std::string(cmdline);
	std::string oldFile = path.substr(path.find_last_of('\\')+1,path.find_last_of('.')-path.find_last_of('\\')-1);
	std::transform(oldFile.begin(), oldFile.end(), oldFile.begin(), ::tolower);
	while(cmd.find("%d")!=cmd.npos) cmd = cmd.replace(cmd.find("%d"),2,path);
	while(cmd.find("%f")!=cmd.npos) cmd = cmd.replace(cmd.find("%f"),2,oldFile);
	while(cmd.find("%o")!=cmd.npos) cmd = cmd.replace(cmd.find("%o"),2,out_dir);
	while(cmd.find("%s")!=cmd.npos) cmd = cmd.replace(cmd.find("%s"),2,sol_dir);
	
	//Check if file exists
	int i1 = cmd.find("Output=")+7;
	int i2 = cmd.find("\"",i1 );
	std::string newFile = cmd.substr(i1, i2-i1);
	std::filebuf ntxFile;
	ntxFile.open(newFile, std::ios::in | std::ios::binary);
	if(ntxFile.is_open())
		return 1;

	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

	if(!CreateProcess(NULL,_strdup(cmd.c_str()),NULL,NULL,false,0,NULL,_strdup(sol_dir.c_str()),&si,&pi)) return 1;

    WaitForSingleObject( pi.hProcess, INFINITE );

	DWORD ec;
	GetExitCodeProcess(pi.hProcess,&ec);

    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	return ec;
}


void writeMeshesToCfg(std::string fname)
{
		std::ifstream stream(fname);
		std::string cfgContent;
		std::string curLine;

		while(std::getline(stream,curLine,'\n'))
		{
			if(curLine.compare(0,4,"Mesh")!=0) cfgContent += curLine+'\n';
		}

		stream.close();

		for each(std::pair<const std::string,Mesh> m in toCfg)
		{
			int c=0;

			if(m.second.Builds["diffuse"].size()==0) m.second.Builds["diffuse"].push_back("-");
			if(m.second.Builds["specular"].size()==0) m.second.Builds["specular"].push_back("-");
			if(m.second.Builds["glow"].size()==0) m.second.Builds["glow"].push_back("-");
			if(m.second.Builds["normal"].size()==0) m.second.Builds["normal"].push_back("-");

			for each(std::string t3d in m.second.Builds["mesh"])
			{
				for each(std::string diff in m.second.Builds["diffuse"])
				{
					for each(std::string spec in m.second.Builds["specular"])
					{
						for each(std::string glow in m.second.Builds["glow"])
						{
							for each(std::string normal in m.second.Builds["normal"])
							{
								std::stringstream ss;
								char count[5] = "";
								if(c++>0) sprintf_s(count,"%d",c);
								ss<<"Mesh "<<m.first<<count<<" "
									<<t3d<<" "
									<<diff<<" "
									<<spec<<" "
									<<glow<<" "
									<<normal<<"\n";
								cfgContent += ss.str();
							}
						}
					}
				}
			}
		}

		std::ofstream output(fname,std::ios_base::trunc);
		output<<cfgContent;
		output.close();
}