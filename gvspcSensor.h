#ifndef GVSPCSENSOR_H
#define GVSPCSENSOR_H

#include <complex>
#include "gvspcPix.h"
#include "gvspcFifo.h"
#include "gvspcV2PM.h"
#include "gvspcLS.h"
#include "gvspcCsv.h"
#include "gvspcErrorCode.h"

class gvspcSensor
{
	const int n_tel;
	const int n_ph;
	const int n_bl;
	
	int n_ch, n_pl;
	std::vector<int> tels[2];
	std::vector<int> bl_sign;
	std::vector<int> bl_arr;
	std::vector<int> ctri[3];
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
	std::vector<double> var_flux;
	// row x col = baseline x spec. channels
	std::vector<std::vector<double> > v_cos_pd;
	std::vector<std::vector<double> > v_sin_pd;
	std::vector<std::vector<double> > var_v_cos_pd;
	std::vector<std::vector<double> > var_v_sin_pd;
	std::vector<double> gd;
	std::vector<double> var_gd;
	std::vector<double> opl;
	
	// for calibration only
	std::vector<gvspcPix> mean_phot;
	std::vector<double> ps;

public:
	gvspcSensor();
	~gvspcSensor();
	
	// checks
	int has_indices();
	int has_ps();
	int has_v2pms();

	// for regular analysis
	int process_image(cpl_image *img, int type);
	int compute_opd(int p, int novar=0);
	int compute_opl();
	double get_flux(int t) const;
	double get_opl(int t) const;
	double get_gd(int i) const;
	double get_pd(int i, int j) const;
	double get_v2(int i, int j) const;
	double get_cp(int c, int j) const;
	double get_var_flux(int t) const;
	double get_var_gd(int i) const;
	double get_var_pd(int i, int j) const;
	double get_var_v2(int i, int j) const;
	int num_baselines() const;
	int num_polstates() const;
	int num_telescopes() const;
	int num_scichannel() const;
	int get_tel(int i, int t) const;
	
	// for calibration only
	int load_pixel_indices(char *csv);
	int save_dark();
	int save_phot(int tel);
	int set_default_ps();
	int set_ps(const std::vector<double>& ps);
	int compute_v2pms();
	int compute_pserr(); // TODO: tbd
	int load_v2pms(const char *filename);
	int save_v2pms(const char *filename);

private:
	void init();
	void reinit();
	int compute_fv(int p, int novar=0);
	int compute_gd();
	void complex_prod(double a[], double b[]);

};

#endif

