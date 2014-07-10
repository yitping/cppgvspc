/*
 */

#include "gvspcCsv.h"

/*
 * Read matrices/arrays stored in csv format into memory (helper function)
 */
void csv2Array_split(char *line, void *val, char *sep, cpl_type t)
{
  const char *tok;
  int i=0;
  for(tok = strtok(line, sep); tok != NULL; tok=strtok(NULL, sep))
  {
    switch (t)
    {
      case CPL_TYPE_DOUBLE:
	sscanf(tok, "%lf", ((double *) val)+i);
	break;
      case CPL_TYPE_INT:
      default:
	sscanf(tok, "%d",  ((int *) val)+i);
    }
    i++;
  }
}


/*
 * Read matrices/arrays stored in csv format into memory
 */
int csv2Array(char *csv, char *var, int nrow, int ncol, void *arr, cpl_type t)
{
  FILE *fp;
  int arrayid=-1, i=0, ni=0, nj=0, sel=9999; /* a big number for sel */
  char line[60000], arrname[50];

  cpl_msg_debug(cpl_func, "Reading matrix %s in %s...", var, csv);
  if ((fp=fopen(csv, "r")) == NULL)
  {
    cpl_msg_error(cpl_func, "cannot open file\n");
    return 1;
  }

  while ((fgets(line, 60000, fp) != NULL) && (sel >= arrayid))
  {
    if (strncmp(line, "#", 1) == 0)
    {
      arrayid += 1; i=0;
      sscanf(line, "%*[# ]%[a-zA-Z0-9_]%*[, ]%d%*[, ]%d", arrname, &ni, &nj);
      cpl_msg_debug(cpl_func, "Reading arrayid=%d (%s:[%d,%d])...", arrayid, arrname, ni, nj);
      if ((strcmp(var, arrname) == 0) && (nrow == ni) && (ncol == nj))
      {
	sel = arrayid;
      }
    }
    else
    {
      if (sel == arrayid)
      {
	switch (t)
	{
	  case CPL_TYPE_DOUBLE:
	    csv2Array_split(line, ((double *) arr)+i*nj, (char *) ",", t);
	    break;
	  case CPL_TYPE_INT:
	  default:
	    csv2Array_split(line, ((int *) arr)+i*nj, (char *) ",", t);
	}
	i++;
      }
    }
  }

  fclose(fp);
  return (sel == 9999) ? 1 : 0;
}

int csv2Dblarr(char *csv, char *var, int nrow, int ncol, double *arr)
{
  return csv2Array(csv, var, nrow, ncol, arr, CPL_TYPE_DOUBLE);
}

int csv2Intarr(char *csv, char *var, int nrow, int ncol, int *arr)
{
  return csv2Array(csv, var, nrow, ncol, arr, CPL_TYPE_INT);
}

/*
 * Write a C array to a file in csv format
 */
int array2Csv(char *csv, char *var, int nrow, int ncol, void *arr, int append, cpl_type t)
{
  int i, j;
  FILE *fileptr;

  fileptr = fopen(csv, (append == 0) ? "w" : "a");
  if (fileptr == NULL)
  {
    cpl_msg_error(cpl_func, "cannot write to file %s.", csv);
    return 1;
  }

  fprintf(fileptr, "# %s, %d, %d\n", var, nrow, ncol);
  for (i=0; i<nrow; i++)
  {
    for (j=0; j<ncol; j++)
    {
      switch (t)
      {
	case CPL_TYPE_DOUBLE:
	  fprintf(fileptr, (j == 0) ? CSV_DBL_FORMAT1 : CSV_DBL_FORMAT2,
	      (double) *(((double *) arr)+(i*ncol+j)));
	  break;
	case CPL_TYPE_INT:
	default:
	  fprintf(fileptr, (j == 0) ? CSV_INT_FORMAT1 : CSV_INT_FORMAT2,
	      (int) *(((int *) arr)+(i*ncol+j)));
      }
    }
    fprintf(fileptr, "\n");
  }

  fclose(fileptr);
  return 0;
}

int dblarr2Csv(char *csv, char *var, int nrow, int ncol, double *arr, int append)
{
  return array2Csv(csv, var, nrow, ncol, arr, append, CPL_TYPE_DOUBLE);
}

int intarr2Csv(char *csv, char *var, int nrow, int ncol, int *arr, int append)
{
  return array2Csv(csv, var, nrow, ncol, arr, append, CPL_TYPE_INT);
}

