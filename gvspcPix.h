#ifndef GVSPCPIX_H
#define GVSPCPIX_H

#include <vector>
#include <cpl.h>
#include "gvspcConst.h"

class gvspcPix
{
	int n_ph, n_bl, n_ch, n_pl;
	std::vector<double> v;
	std::vector<double> y; // Y-junction

	public:
	gvspcPix();
	gvspcPix(int n_ch, int n_pl);
	gvspcPix(const gvspcPix& other_pix);
	~gvspcPix();

	// prepare internal array
	void resize(int n_ch, int n_pl);

	// operators
	gvspcPix& operator=(const gvspcPix& other_pix);
	double operator[](long i);
	double operator[](long i) const;
	gvspcPix operator+(const gvspcPix& B) const;
	gvspcPix operator*(double b) const;
	gvspcPix operator*(const gvspcPix& B) const;
	gvspcPix operator/(double b) const;
	gvspcPix operator/(const gvspcPix& B) const;

	// get essential array size
	int isNull() const;
	int num_ch() const;
	int num_pl() const;
	long  size() const;
	
	// set the value of each pixel
	int set(cpl_image *img, int *idx, int cnr_ch, int cnr_io);
	int set(double val, int k, int i, int j, int p);

	// get the value of each pixel
	double get(int k, int i, int j, int p) const;
	double get(long i) const;

	// sum over v and y
	double sum();

	private:
	void init(int n_ch=0, int n_pl=0);
	void copy(const gvspcPix& other_pix);
	long convert_img_indices(int l, int j, int p, int isYjunc=0);

};

#endif

