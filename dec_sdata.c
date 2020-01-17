#include <stdio.h>
#include <stdlib.h>
#include "dec_sdata.h"

#define TBL_SZ 0x10000

/* Decrypts the public certificate list at http://dtv-p.c-ij.com/sdata/struct01/sdata.bin
 * (versioned by http://dtv-p.c-ij.com/sdata/struct01/version.bin )
 */

void decode_sdata(unsigned char *in)
{
	int j, k;

	for (j=0, k=0; j<TBL_SZ; j+=16)
	{
		for (k=0; k<16 && xor_tab[j+k]==in[k]; k++);
		if (k==16) break;
	}
	if (j<TBL_SZ)
	{
		for (k=0; k<TBL_SZ; k++) in[k]^=xor_tab[j+k];
	}
	else fprintf (stderr, "Failed\n");
}

int usage(char **argv)
{
	// -o 0xEC4FB8 dataout.bin encdata.out
	printf ("%s [-o 0xOffset] <Input file> <Output file>\n", argv[0]);
	return 1;
}

int main(int argc, char **argv)
{
	FILE *fp, *fpout;
	int i=1, offset=0;
	unsigned char *buf;

	printf ("Canon PIXMA sdata decrypter V1.00\nleecher@dose.0wnz.at 10/2019\n\n");
	if (argc<3) return usage(argv);

	if (!strcmp(argv[1], "-o"))
	{
		if (argc<5) return usage(argv);
		if (!sscanf(argv[++i], "%x", &offset))
		{
			fprintf(stderr, "Cannot parse offset %s as hex\n", argv[i]);
			return 1;
		}
		i++;
	}
	if (!(fp=fopen(argv[i], "rb")))
	{
		perror("Cannot open input file");
		return 1;
	}
	if (!(fpout=fopen(argv[++i], "wb")))
	{
		fclose(fp);
		perror("Cannot open output file");
		return 1;
	}
	if (buf=malloc(TBL_SZ))
	{
		if (offset) fseek(fp, offset, SEEK_SET);
		fread(buf, TBL_SZ, 1, fp);
		fclose(fp);
		decode_sdata(buf);
		fwrite(buf, TBL_SZ, 1, fpout);
		fclose(fpout);
		free(buf);
	} else {
		perror("malloc");
		return 1;
	}
	return 0;
}
