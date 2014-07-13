//
//  gvspcCsv.cpp
//  cppgvspc
//
//  Created by Yitping Kok on 7/13/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "gvspcCsv.h"

gvspcCsv::gvspcCsv()
{
	init();
	std::cout << "1 gvspcCsv created" << std::endl;
}

gvspcCsv::gvspcCsv(const char *filename, int rw)
{
	link_to(filename, rw);
	std::cout << "1 gvspcCsv (linked) created" << std::endl;
}

gvspcCsv::~gvspcCsv()
{
	std::cout << "1 gvspcCsv destroyed" << std::endl;
}

int gvspcCsv::link_to(const char *filename, int rw)
{
	std::ifstream fin(filename, (rw == 0) ? std::ios::in : std::ios::out);
	if (!fin.is_open())
	{
		std::cerr << "cannot open file: " << filename << std::endl;
		fin.close();
		return 0;
	}
	this->filename = filename;

	std::string line, label, delim;
	long lnum=0, m, n;

	while (std::getline(fin, line))
	{
		if (line.rfind('#') != std::string::npos)
		{
			std::stringstream hdr(line);
			std::getline(hdr, label, ','); hdr >> m >> delim >> n;
			labels.push_back(label.substr(2));
			nrows.push_back(m);
			ncols.push_back(n);
			lnums.push_back(lnum);
		}
		lnum++;
	}

	file_linked = true;

	fin.close();
	return 0;
}

int gvspcCsv::has_variables() { return labels.size(); }
bool gvspcCsv::has_linked_file() { return file_linked; }

void gvspcCsv::info()
{
	std::cout << "File: " << filename << std::endl;
	std::cout << "Names" << "\t";
	std::cout << "Line#" << "\t";
	std::cout << "Rows"  << "\t";
	std::cout << "Cols"  << std::endl;
	for (int i=0; i<labels.size(); i++)
	{
		std::cout << labels[i] << "\t"; 
		std::cout << lnums[i]  << "\t"; 
		std::cout << nrows[i]  << "\t"; 
		std::cout << ncols[i]  << std::endl;
	}
}

int gvspcCsv::read_as_dbl(const char *label, std::vector<std::vector<double> >& data)
{
	int vnum = get_varnum(label);
	return read_as_dbl(vnum, data);
}

int gvspcCsv::read_as_dbl(int vnum, std::vector<std::vector<double> >& data)
{
	if (read(vnum) < 0) return -1;
	data.clear();
	data.resize(nrows[vnum]);
	for (int i=0; i<content.size(); i++)
	{
		data[i].resize(ncols[vnum],0);
		for (int j=0; j<content[i].size(); j++) data[i][j] = stod(content[i][j]);
	}
	return vnum;
}

int gvspcCsv::read_as_int(const char *label, std::vector<std::vector<int> >& data)
{
	int vnum = get_varnum(label);
	return read_as_int(vnum, data);
}

int gvspcCsv::read_as_int(int vnum, std::vector<std::vector<int> >& data)
{
	if (read(vnum) < 0) return -1;
	data.clear();
	data.resize(nrows[vnum]);
	for (int i=0; i<content.size(); i++)
	{
		data[i].resize(ncols[vnum],0);
		for (int j=0; j<content[i].size(); j++) data[i][j] = stoi(content[i][j]);
	}
	return vnum;
}

int gvspcCsv::debugme(const char *fname)
{
	std::cout << "linenum " << fname << " = " << get_varnum(fname) << std::endl;
	return 0;
}

///// private /////

void gvspcCsv::init()
{
	file_linked = false;
}

int gvspcCsv::read(int vnum)
{
	if ((vnum < 0) || (vnum > labels.size()))
	{
		std::cerr << "variable number out of bound" << std::endl;
		return -1;
	}

	if (!content.empty()) content.clear();

	std::ifstream fin(filename, std::ios::in);
	if (!fin.is_open())
	{
		std::cerr << "unexpected error: cannot open file: " << filename << std::endl;
		fin.close();
		return -1;
	}

	int i, j;
	std::string line, datum;
	content.resize(nrows[vnum]);

	// skip unwanted lines
	for (i=0; i<(lnums[vnum]+1); i++) std::getline(fin, line);
	for (i=0; i<nrows[vnum]; i++)
	{
		std::getline(fin, line);
		std::stringstream ss(line);
		content[i].resize(ncols[vnum]);
		for (j=0; j<ncols[vnum]; j++)
		{
			std::getline(ss, datum, ',');
			content[i][j] = datum;
		}
	}

	fin.close();
	return vnum;
}

int gvspcCsv::get_varnum(const char *label)
{
	if (labels.empty()) return -1;
	std::string req(label);
	std::cout << "Checking for " << req << std::endl;
	for (int l=0; l<labels.size(); l++)
	{
		std::cout << labels[l] << std::endl;
		if (req == labels[l]) return l;
	}
	return -1;
}

