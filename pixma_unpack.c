#include <stdio.h>
#include <string.h>

static unsigned char sig[] = 
{
  0x70, 0xB5, 0x05, 0x4C, 0x05, 0x48, 0x06, 0x49, 
  0x45, 0x1A, 0x0E, 0x46, 0x2A, 0x46, 0x31, 0x46, 
  0x20, 0x46, 0xFF
};


// Some sort of compression, was unable to identify the algorithm, LZ4 or something..?
void decompress(unsigned char *src, unsigned char *dest, unsigned int size)
{
	unsigned int i, l, x;
	unsigned char c, d, e, f, *pdst;

	for (x=0; x<size; x++)
	{
		c = *src++;
		l = c & 3; 
		if (!l) l = *src++;
		d = c >> 4;
		if (!d) d = *src++;
		for (i = l - 1; i; i--) *dest++ = *src++;
		if (d)
		{
			e = *src++;
			f = c << 4 >> 6;
			if (f == 3) f = *src++;
			pdst = dest-e-256*f;
			for (l=d+1; l>=0; l--) *dest++ = *pdst++;
		}
	}
}

// Same as above, but with file pointer so that file doesn't need to be mapped to memory
void decompress_fp(FILE *fpIn, FILE *fpOut, unsigned int size)
{
	int c, e, l, d;
	unsigned int f;
	unsigned char buf[260];

	while((c = fgetc(fpIn))!=EOF && ftell(fpOut)<size)
	{
		l = c & 3;
		if (!l) l = fgetc(fpIn);
		d = c >> 4;
		if (!d) d = fgetc(fpIn);
		fread(buf, l-1, 1, fpIn);
		fwrite(buf, l-1, 1, fpOut);
		if (d)
		{
			e = fgetc(fpIn);
			f = (unsigned int)(c << 28) >> 30;
			if (f == 3) f = (unsigned char)fgetc(fpIn);
			fseek(fpOut, 0-e-256*f, SEEK_CUR);
			fread(buf, d+2, 1, fpOut);
			fseek(fpOut, 0, SEEK_END);
			fwrite(buf, d+2, 1, fpOut);
		}
	}
}

int usage(char **argv)
{
	printf ("%s <Input file> <Output file>\n", argv[0]);
	return 1;
}

int search_data(FILE *fp, unsigned char *needle, int len)
{
	unsigned char buf[4096], *p;
	int r;

	for (r=sizeof(buf); r==sizeof(buf) && (r=fread(buf, 1, sizeof(buf), fp));)
	{
		// memmem is so broken that we have to do this manually...
		if ((p=(unsigned char*)memchr(buf, *needle, r)) && p-buf+len<=r && memcmp(p, needle, len)==0)
		{
			fseek(fp, (r*-1)+(p-buf)+len+5, SEEK_CUR);
			return 1;
		}
		fseek(fp, -len+1, SEEK_CUR);
	}
	return 0;
}

int main(int argc, char **argv)
{
	FILE *fp, *fpout;
	long offset=0, size=0;
	int ret=1;

	printf ("Canon PIXMA Firmware unpacker V1.00\nleecher@dose.0wnz.at 10/2019\n\n");
	if (argc<3) return usage(argv);

	if (!(fp=fopen(argv[1], "rb")))
	{
		perror("Cannot open input file");
		return 1;
	}
	if (!(fpout=fopen(argv[2], "w+b")))
	{
		fclose(fp);
		perror("Cannot open output file");
		return 1;
	}

	if (!offset || !size)
	{
		unsigned int end, start;

		if (!search_data(fp, sig, sizeof(sig)))
		{
			fprintf(stderr, "Signature for decompression function not found.\n");
			goto done;
		}
		fread(&offset, 4, 1, fp);
		fread(&end, 4, 1, fp);
		fread(&start, 4, 1, fp);
		size=end-start;
		printf ("Data offset: %08lX\nStart address %08X\nEnd address %08X\nSize: %ld bytes\n\n", offset, start, end, size);
		offset &= 0xFFFFFF;
	}

	fseek(fp, offset, SEEK_SET);
	printf ("Decompressing...");
	fflush(stdout);
	decompress_fp(fp, fpout, size);
	printf ("Done.\n");
	ret = 0;

done:
	fclose(fpout);
	fclose(fp);
	return ret;
}
