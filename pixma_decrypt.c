#include <stdio.h>
#include <errno.h>

/* Quick and dirty, known plaintext... */
void get_key_simple(unsigned char *in, unsigned char *key)
{
	char assume_1[6]={'S','F','0','9','0','0'};
	char assume_2[10] = {0x0D,0x0A,'S','F','0','5','0','0','0','0'};
	int i, j;

	for (i=0; i<sizeof(assume_1); i++) key[i]=in[i]^assume_1[i];
	for (j=0; j<sizeof(assume_2); j++) key[i+j]=in[i+0x10+j]^assume_2[j];
}

void print_key(unsigned char *key)
{
	int i;

	printf("Key: ");
	for (i=0; i<16; i++)
		printf ("%02X ", key[i]);
	printf("\n");
}

void decrypt(FILE *fpin, FILE *fpout, unsigned char *key)
{
	int len;
	unsigned long sz;
	unsigned int i, j;
	unsigned char buf[32768];
	int c;

	fseek(fpin, 0, SEEK_END);
	sz=ftell(fpin);
	rewind(fpin);
	for(j=0; (len=fread(buf, 1, sizeof(buf), fpin)); j++)
	{
		for(i=0;i<len; i++) buf[i]^=key[i%16];
		fwrite(buf, len, 1, fpout);
		printf("\rDecrypting...%03d%%", (int)(((double)(j*sizeof(buf))/(double)sz)*100));
		fflush(stdout);
	}
	printf ("\rDecryption finished.\n");
}

int usage(char **argv)
{
	printf ("%s <Input file> <Output file>\n", argv[0]);
	return 1;
}

int main(int argc, char **argv)
{
	FILE *fp, *fpout;
	unsigned char buf[32], key[16]={0};

	printf ("Canon PIXMA Firmware decrypter V1.00\nleecher@dose.0wnz.at 10/2019\n\n");
	if (argc<3) return usage(argv);

	if (!(fp=fopen(argv[1], "rb")))
	{
		perror("Cannot open input file");
		return 1;
	}
	if (!(fpout=fopen(argv[2], "wb")))
	{
		fclose(fp);
		perror("Cannot open output file");
		return 1;
	}

	fread(buf, 32, 1, fp);
	get_key_simple(buf, key);
	print_key(key);
	decrypt(fp, fpout, key);

	fclose(fpout);
	fclose(fp);
	return 0;
}
