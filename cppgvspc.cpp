//
//  cppgvspc.cpp
//  cppgvspc
//
//  Created by Yitping Kok on 7/13/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include <cpl.h>
#include <unistd.h>
#include <string>
#include "gvspcSensor.h"

#define SCI_FITS_EXT "IMAGING_DATA_SPE"

int main(int argc, char **argv)
{
  int retval=EXIT_SUCCESS, errflag=0, optflag=0, msgflag=0, opt, selpol=0, num_dark=0, i, j, t, extid;
  long fnum=0;
  char *file_index=NULL, *file_p2vm=NULL, **files_dark=NULL, *pfile=NULL,*file_out=NULL;//, outstr[2048];
  cpl_image *image;
	//  double mean_total_flux[NUM_TELESCOPES];
	//  double mean_opl[NUM_TELESCOPES];
	//  double mean_gd[NUM_BASELINES];
	//  double mean_pd[NUM_BASELINES];
	//  double mean_v2[NUM_BASELINES];
	std::ofstream fout;
	
  cpl_init(CPL_INIT_DEFAULT);
	
  while ((opt = getopt(argc, argv, ":i:p:d:o:a:m")) != -1)
  {
    switch (opt)
    {
      case 'i':
				file_index = optarg;
				break;
      case 'p':
				file_p2vm = optarg;
				break;
      case 'o':
				file_out = optarg;
				break;
      case 'a':
				sscanf(optarg, "%d", &selpol);
				break;
      case 'm':
				msgflag = 1;
				break;
      case 'd':
				i = optind-1;
				files_dark = &argv[i];
				if (optflag++) { errflag++; break; }
				while (i < argc)
				{
					pfile = argv[i++];
					if (pfile[0] != '-') num_dark++;
					else break;
				}
				optind = i-1;
				break;
      case ':':
				fprintf(stderr, "option -%c requires an operand\n", optopt);
				errflag++;
				break;
      case '?':
				fprintf(stderr, "unrecognized option: -%c\n", optopt);
				errflag++;
				break;
    }
  }
  
  cpl_msg_set_level((msgflag) ? CPL_MSG_DEBUG : CPL_MSG_INFO);
	
  if (errflag || (file_index == NULL) || (file_p2vm == NULL) ||
      (num_dark == 0))
  {
		std::cerr << "Usage: " << argv[0] << " -i index -p p2vm -d dark_fits1 [dark_fits2 ...] [-m -a sel_pol -o out_csv] fits ..." << std::endl;
    retval = EXIT_FAILURE;
  }
  else
  {
    cpl_msg_debug(cpl_func, "index file: %s", file_index);
    cpl_msg_debug(cpl_func, "p2vm file: %s", file_p2vm);
    for (i=0; i<num_dark; i++) cpl_msg_debug(cpl_func, "dark file: %s", files_dark[i]);
    if (file_out == NULL) cpl_msg_debug(cpl_func, "out file: <stdout>");
    else
    {
			fout.open(file_out, std::ios::out);
      if (!fout.is_open())
      {
				cpl_msg_warning(cpl_func, "cannot write to file %s.", file_out);
				cpl_msg_debug(cpl_func, "out file: <stdout>");
      }
      else
			{
				fout.setf(std::ios::fixed, std::ios::floatfield);
				cpl_msg_debug(cpl_func, "out file: %s", file_out);
			}
    }

    // instantiate and init sensor
		gvspcSensor sensor;
		sensor.load_pixel_indices(file_index);
		
		if (sensor.has_indices())
		{
			sensor.set_default_ps();
			sensor.load_v2pms(file_p2vm);
			
			cpl_msg_info(cpl_func, "reading dark fits...");
			for (i=0; i<num_dark; i++)
			{
				if (access(files_dark[i], R_OK) != 0)
				{
					cpl_msg_error(cpl_func, "file not found: %s\n", files_dark[i]);
					continue;
				}
				extid = (cpl_fits_count_extensions(files_dark[i]) == 1) ? 0 :cpl_fits_find_extension(argv[optind], (char *) SCI_FITS_EXT);
				cpl_msg_info(cpl_func, "loading %s [%d]", files_dark[i], extid);
				j = 0;
				while (1)
				{
					image = cpl_image_load(files_dark[i], CPL_TYPE_DOUBLE, j++, extid);
					if (image == NULL) break;
					sensor.process_image(image, 0);
					cpl_image_delete(image);
				}
			}
			sensor.save_dark();
			
			if (fout.is_open())
			{
				fout << "# " << std::setw(10) << "fnum";
				fout << std::setw(5) << "fr";
				for (i=0; i<sensor.num_baselines(); i++)
					fout << std::setw(12-3) << "gd[" << sensor.get_tel(i,0) << sensor.get_tel(i,1) << "]";
				for (i=0; i<sensor.num_baselines(); i++)
					fout << std::setw(12-3) << "v2[" << sensor.get_tel(i,0) << sensor.get_tel(i,1) << "]";
				for (i=0; i<sensor.num_baselines(); i++)
					fout << std::setw(12-3) << "pd[" << sensor.get_tel(i,0) << sensor.get_tel(i,1) << "]";
				for (t=0; t<sensor.num_telescopes(); t++)
					fout << std::setw(12-2) << "flux[" << t << "]";
				fout << std::endl;
			}
			
			for (; optind<argc; optind++)
			{
				if (access(argv[optind], R_OK) != 0)
				{
					cpl_msg_error(cpl_func, "file not found: %s\n", argv[optind]);
					continue;
				}
				extid = (cpl_fits_count_extensions(argv[optind]) == 1) ? 0 :
				cpl_fits_find_extension(argv[optind], (char *) SCI_FITS_EXT);
				cpl_msg_info(cpl_func, "loading %s [%d]", argv[optind], extid);
				j = 0;
				while (1)
				{
					image = cpl_image_load(argv[optind], CPL_TYPE_DOUBLE, j++, extid);
					if (image == NULL) break;
					sensor.process_image(image, 1);
					cpl_image_delete(image);
					
					sensor.compute_fv(0);
					sensor.compute_gd();
				
//				/* compute all relevant data */
//				gvspc_data_compute_gd(mean_gd, mean_pd, mean_v2, 1);
//				gvspc_data_compute_total_flux(mean_total_flux);
//				gvspc_data_compute_opl(mean_opl);
					
//				/*
//				 for (i=0; i<NUM_TELESCOPES; i++)
//				 cpl_msg_debug(cpl_func, "flux[%d]: %10.0f", i+1, mean_total_flux[i]);
//				 for (i=0; i<NUM_BASELINES; i++)
//				 cpl_msg_debug(cpl_func, "gd[%d]: %4.5f", i+1, mean_gd[i]);
//				 */
//				output_format(outstr, fnum, j-1,
//											mean_gd, 6,
//											mean_v2, 6,
//											mean_pd, 6,
//											mean_total_flux, 4);
//				fprintf((fptr == NULL) ? stdout : fptr, "%s\n", outstr);
					
					if (fout.is_open())
					{
						fout << std::setw(12) << fnum;
						fout << std::setw(5)  << j-1;
						for (i=0; i<sensor.num_baselines(); i++)
							fout << std::setprecision(5) << std::setw(12) << sensor.get_gd(i);
						for (i=0; i<sensor.num_baselines(); i++)
							fout << std::setprecision(5) << std::setw(12) << sensor.get_v2(i, sensor.num_scichannel()/2);
						for (i=0; i<sensor.num_baselines(); i++)
							fout << std::setprecision(5) << std::setw(12) << sensor.get_pd(i, sensor.num_scichannel()/2);
						for (t=0; t<sensor.num_telescopes(); t++)
							fout << std::setprecision(0) << std::setw(12) << sensor.get_flux(t);
						fout << std::endl;
 					}
				}
				fnum++;
			}
		}
  }
	
  if (!fout.is_open()) fout.close();
  if (!cpl_memory_is_empty()) cpl_msg_error(cpl_func, "memory is not empty.");
  cpl_end();
  return retval;
}