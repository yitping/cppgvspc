#ifndef GVSPCSENSOR_H
#define GVSPCSENSOR_H

#include <vector>
#include <array>
#include "gvspcConst.h"
#include "gvspcPix.h"
#include "gvspcFifo.h"

class gvspcSensor
{
	std::vector<int> pixel_index;
	int n_ch, n_pl, corner_ch, corner_io;
	double bw;
	
	// for regular analysis
	gvspcFifo<gvspcPix> fifo_pix_flux;
	gvspcFifo<gvspcPix> fifo_pix_dark;
	gvspcPix mean_dark;
	gvspcPix read_noiz;
	
	// for photometric calibration only
	std::array<gvspcPix,NUM_TELESCOPES> mean_phot;

	public:
	gvspcSensor();
	~gvspcSensor();

	int isNull();

	int load_pixel_indices(char *csv);
	int process_image(cpl_image *img, int type);
	int save_dark();
	int save_phot();
	int compute_v2pm();


	// for debugging purpose
	void dump_a_pix();
	void dump_an_index();

	private:
	void init();

};

#endif

