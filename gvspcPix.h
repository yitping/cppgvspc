#ifndef GVSPCPIX_H
#define GVSPCPIX_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cpl.h>

#define NUM_TELESCOPES		4
#define MAX_PHASE_SHIFTS	4
#define NUM_BASELINES			0.5*NUM_TELESCOPES*(NUM_TELESCOPES-1)
#define NUM_IO_OUTPUT			23

class gvspcPix
{
	const int n_ph, n_bl;
	int n_ch, n_pl;
	std::vector<double> v;
	std::vector<double> y; // Y-junction
	std::vector<double> v_jp;

public:
	gvspcPix();
	gvspcPix(int n_ch, int n_pl);
	gvspcPix(const gvspcPix& B);
	gvspcPix(const gvspcPix& B, int j, int p);
	~gvspcPix();

	// prepare internal array
	void resize(int n_ch, int n_pl);

	// operators
	gvspcPix& operator=(const gvspcPix& B);
	double operator[](long i);
	double operator[](long i) const;
	gvspcPix operator+(const gvspcPix& B) const;
	gvspcPix operator-(const gvspcPix& B) const;
	gvspcPix operator*(double b) const;
	gvspcPix operator*(const gvspcPix& B) const;
	gvspcPix operator/(double b) const;
	gvspcPix operator/(const gvspcPix& B) const;
	gvspcPix& operator+=(double b);
	gvspcPix& operator+=(const gvspcPix& B);
	gvspcPix& operator*=(double b);
	gvspcPix& operator*=(const gvspcPix& B);
	gvspcPix& operator/=(double b);
	gvspcPix& operator/=(const gvspcPix& B);
	
	// get essential array size
	int isNull() const;
	int num_ph() const;
	int num_bl() const;
	int num_ch() const;
	int num_pl() const;
	long  size() const;
	
	// set the value of each pixel
	int set(cpl_image *img, int *idx, int cnr_ch, int cnr_io);
	int set(double val, int k, int i, int j, int p);
	gvspcPix abs();

	// get the value of each pixel
	double get(int k, int i, int j, int p);
	double get(long i);
	
	// get the all pixels or an area of the pixels
	const std::vector<double>& data();
	const std::vector<double>& data(int j, int p);

	// sum over v and y
	double sum(int p=0);

private:
	void init(int n_ch, int n_pl);
	void copy(const gvspcPix& B);
	void copy(const gvspcPix& B, int j, int p);
	long convert_img_indices(int l, int j, int p, int isYjunc=0);
	long convert_4D_indices(int k, int i, int j, int p, int isYjunc=0);

};

#endif

