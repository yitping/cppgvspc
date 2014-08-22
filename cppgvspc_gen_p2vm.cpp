#include <iostream>
#include <cpl.h>
#include <unistd.h>
#include <string.h>
#include "gvspcSensor.h"

#define SCI_FITS_EXT "IMAGING_DATA_SPE"

int main(int argc, char **argv)
{
	int retval=EXIT_SUCCESS, errflag=0, optflag[5]={0}, msgflag=0, opt, i, j, extid;
  char **files_dark, **files_tel1, **files_tel2, **files_tel3, **files_tel4, *pfile, *file_index, *file_v2pm, *file_ps;
  int num_dark=0, num_tel1=0, num_tel2=0, num_tel3=0, num_tel4=0;
	int max_frames=10;
  cpl_image *image;
	
  cpl_init(CPL_INIT_DEFAULT);
	
	file_index = NULL;
	file_ps    = NULL;
  file_v2pm  = NULL;
  files_dark = NULL;
  files_tel1 = NULL;
  files_tel2 = NULL;
  files_tel3 = NULL;
  files_tel4 = NULL;
	
  while ((opt = getopt(argc, argv, ":i:s:d:1:2:3:4:p:mn:")) != -1)
  {
		switch (opt)
		{
      case 'i':
				file_index = optarg;
				break;
			case 's':
				file_ps = optarg;
				break;
      case 'p':
				file_v2pm = optarg;
				break;
      case 'm':
				msgflag = 1;
				break;
			case 'n':
				sscanf(optarg, "%d", &max_frames);
				break;
      case 'd':
      case '1':
      case '2':
      case '3':
      case '4':
				i = optind-1;
				/* just one pass for each */
				if (opt == 'd')
				{
					files_dark = &argv[i];
					if (optflag[0]++) { errflag++; break; }
				}
				if (opt == '1')
				{
					files_tel1 = &argv[i];
					if (optflag[1]++) { errflag++; break; }
				}
				if (opt == '2')
				{
					files_tel2 = &argv[i];
					if (optflag[2]++) { errflag++; break; }
				}
				if (opt == '3')
				{
					files_tel3 = &argv[i];
					if (optflag[3]++) { errflag++; break; }
				}
				if (opt == '4')
				{
					files_tel4 = &argv[i];
					if (optflag[4]++) { errflag++; break; }
				}
				while (i < argc)
				{
					pfile = argv[i++];
					if (pfile[0] != '-')
					{
						if (opt == 'd') num_dark++;
						if (opt == '1') num_tel1++;
						if (opt == '2') num_tel2++;
						if (opt == '3') num_tel3++;
						if (opt == '4') num_tel4++;
					}
					else break;
				}
				optind = i-1;
				break;
      case ':':
				std::cerr << "option -" << optopt << " requires an operand" << std::endl;
				errflag++;
				break;
      case '?':
				std::cerr << "unrecognized option: -" << optopt << std::endl;
				errflag++;
				break;
    }
  }
	
	cpl_msg_set_level((msgflag) ? CPL_MSG_DEBUG : CPL_MSG_INFO);
	
	if (errflag || (file_index == NULL) || (num_dark == 0) ||
      (num_tel1 == 0) || (num_tel2 == 0) || (num_tel3 == 0) || (num_tel4 == 0))
  {
		std::cerr << "Usage: " << argv[0] << " -i index [-s ps -p p2vm -m -n max_frames]\n\t-d dark_fits1 [dark_fits2 ...]\n\t-1 tel1_fits1 [tel1_fits2 ...]\n\t-2 tel2_fits1 [tel2_fits2 ...]\n\t-3 tel3_fits1 [tel3_fits2 ...]\n\t-4 tel4_fits1 [tel4_fits2 ...]\n" << std::endl;
    retval = EXIT_FAILURE;
  }
  else
  {
		if (file_v2pm == NULL) file_v2pm = (char *) "v2pms.csv";
    for (i=0; i<num_dark; i++) cpl_msg_info(cpl_func, "dark file: %s", files_dark[i]);
    for (i=0; i<num_tel1; i++) cpl_msg_info(cpl_func, "tel1 file: %s", files_tel1[i]);
    for (i=0; i<num_tel2; i++) cpl_msg_info(cpl_func, "tel2 file: %s", files_tel2[i]);
    for (i=0; i<num_tel3; i++) cpl_msg_info(cpl_func, "tel3 file: %s", files_tel3[i]);
    for (i=0; i<num_tel4; i++) cpl_msg_info(cpl_func, "tel4 file: %s", files_tel4[i]);
    cpl_msg_info(cpl_func, "p2vm file: %s", file_v2pm);
		
		// instantiate and init sensor
		gvspcSensor sensor;
		sensor.load_pixel_indices(file_index);
		
		if (sensor.has_indices())
		{
			cpl_msg_info(cpl_func, "reading dark fits...");
			for (i=0; i<num_dark; i++)
			{
				extid = (cpl_fits_count_extensions(files_dark[i]) == 1) ? 0 :cpl_fits_find_extension(files_dark[i], (char *) SCI_FITS_EXT);
				cpl_msg_info(cpl_func, "loading %s [%d]", files_dark[i], extid);
				j = 0;
				while (1 && (j < max_frames))
				{
					image = cpl_image_load(files_dark[i], CPL_TYPE_DOUBLE, j++, extid);
					if (image == NULL) break;
					sensor.process_image(image, 0);
					cpl_image_delete(image);
				}
			}
			sensor.save_dark();
			
			cpl_msg_info(cpl_func, "reading tel1 fits...");
			for (i=0; i<num_tel1; i++)
			{
				extid = (cpl_fits_count_extensions(files_tel1[i]) == 1) ? 0 :cpl_fits_find_extension(files_tel1[i], (char *) SCI_FITS_EXT);
				cpl_msg_info(cpl_func, "loading %s [%d]", files_tel1[i], extid);
				j = 0;
				while (1)
				{
					image = cpl_image_load(files_tel1[i], CPL_TYPE_DOUBLE, j++, extid);
					if (image == NULL) break;
					sensor.process_image(image, 1);
					cpl_image_delete(image);
				}
			}
			sensor.save_phot(1);
			
			cpl_msg_info(cpl_func, "reading tel2 fits...");
			for (i=0; i<num_tel2; i++)
			{
				extid = (cpl_fits_count_extensions(files_tel2[i]) == 1) ? 0 :cpl_fits_find_extension(files_tel2[i], (char *) SCI_FITS_EXT);
				cpl_msg_info(cpl_func, "loading %s [%d]", files_tel2[i], extid);
				j = 0;
				while (1 && (j < max_frames))
				{
					image = cpl_image_load(files_tel2[i], CPL_TYPE_DOUBLE, j++, extid);
					if (image == NULL) break;
					sensor.process_image(image, 1);
					cpl_image_delete(image);
				}
			}
			sensor.save_phot(2);
			
			cpl_msg_info(cpl_func, "reading tel3 fits...");
			for (i=0; i<num_tel3; i++)
			{
				extid = (cpl_fits_count_extensions(files_tel3[i]) == 1) ? 0 :cpl_fits_find_extension(files_tel3[i], (char *) SCI_FITS_EXT);
				cpl_msg_info(cpl_func, "loading %s [%d]", files_tel3[i], extid);
				j = 0;
				while (1 && (j < max_frames))
				{
					image = cpl_image_load(files_tel3[i], CPL_TYPE_DOUBLE, j++, extid);
					if (image == NULL) break;
					sensor.process_image(image, 1);
					cpl_image_delete(image);
				}
			}
			sensor.save_phot(3);
			
			cpl_msg_info(cpl_func, "reading tel4 fits...");
			for (i=0; i<num_tel4; i++)
			{
				extid = (cpl_fits_count_extensions(files_tel4[i]) == 1) ? 0 :cpl_fits_find_extension(files_tel4[i], (char *) SCI_FITS_EXT);
				cpl_msg_info(cpl_func, "loading %s [%d]", files_tel4[i], extid);
				j = 0;
				while (1 && (j < max_frames))
				{
					image = cpl_image_load(files_tel4[i], CPL_TYPE_DOUBLE, j++, extid);
					if (image == NULL) break;
					sensor.process_image(image, 1);
					cpl_image_delete(image);
				}
			}
			sensor.save_phot(4);

			if (file_ps != NULL) sensor.load_ps(file_ps);
			sensor.compute_v2pms();
			sensor.save_v2pms(file_v2pm);
			
		}
		
	}
	
	if (!cpl_memory_is_empty()) cpl_msg_error(cpl_func, "memory is not empty.");
	cpl_end();
	return retval;
}
