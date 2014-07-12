#ifndef GVSPCSENSOR_H
#define GVSPCSENSOR_H

#include <vector>
#include "gvspcConst.h"
#include "gvspcPix.h"
#include "gvspcFifo.h"
#include "gvspcV2PM.h"

class gvspcSensor
{
	const int n_tel;
	const int n_ph;
	const int n_bl;
	
	int n_ch, n_pl;
	std::vector<int> tels[2];
	std::vector<int> pixel_index;
	int corner_ch, corner_io;
	double bw;
	
	bool indices_ready;
	bool ps_ready;
	bool v2pms_ready;
	
	// for regular analysis
	gvspcFifo<gvspcPix> fifo_pix_flux;
	gvspcFifo<gvspcPix> fifo_pix_dark;
	gvspcPix mean_dark;
	gvspcPix var_dark;
	std::vector<gvspcV2PM> v2pms;
	std::vector<double> flux;
	
	// for calibration only
	std::vector<gvspcPix> mean_phot;
	std::vector<double> ps;
	
	
	// gd 6
	// pd 6 x n_ch
	// v2 6 x n_ch
	// fl 4
	// opl 4
	// v2pm 24 x 16 x n_ch
	// lm 1
	

public:
	gvspcSensor(int m=4, int n=4);
	~gvspcSensor();

	// checks
	int has_indices();
	int has_ps();
	int has_v2pms();

	// for regular analysis
	int process_image(cpl_image *img, int type);
	
	// for calibration only
	int load_pixel_indices(char *csv);
	int save_dark();
	int save_phot(int tel);
	int set_default_ps();
	int set_ps(const std::vector<double>& ps);
	int compute_v2pms();
	int compute_pserr(); // spare
	int load_v2pms_from_file(const char *);
	int save_v2pms_to_file(const char *);

	// for debugging purpose
	void dump_a_pix();
	void dump_an_index();

private:
	void init();
	void reinit();

};

#endif

