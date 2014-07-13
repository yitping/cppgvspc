//
//  gvspcCsv.h
//  cppgvspc
//
//  Created by Yitping Kok on 7/13/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#ifndef cppgvspc_gvspcCsv_h
#define cppgvspc_gvspcCsv_h

#include <fstream>
#include <string>
#include <vector>

class gvspcCsv
{
	std::string filename;
	std::vector<std::string> labels;
	std::vector<long> nrows;
	std::vector<long> ncols;
	std::vector<long> lnums;
	std::vector<std::vector<std::string> > content;
	bool file_linked;
	
public:
	gvspcCsv();
	gvspcCsv(const char *filename, int rw=0);
	~gvspcCsv();
	
	// checks
	int has_variables();
	bool has_linked_file();
	
	// compliments the default constructor
	int link_to(const char *filename, int rw=0);
	
	// read functions
	// TODO: there should be a better way but I'm oot.
	int read_as_dbl(const char *label, std::vector<std::vector<double> >& data);
	int read_as_dbl(int vnum, std::vector<std::vector<double> >& data);
	int read_as_int(const char *label, std::vector<std::vector<int> >& data);
	int read_as_int(int vnum, std::vector<std::vector<int> >& data);
	
	// write functions
	int write(const char *label, const std::vector<std::vector<double> >& data);
	int write(const char *label, const std::vector<std::vector<int> >& data);
	
	void info();
	
	int debugme(const char *name);
	
private:
	void init();
	int read(const char *label);
	int read(int vnum);
	int get_varnum(const char *label);
	
};

#endif
