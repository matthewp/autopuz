#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#define ROWSIZE 256

FILE *infile;
int width, height, j, k, ccount, mcount, anum, dnum;
int apos, dpos;
int solve;

char inbuf[ROWSIZE];

/* Here is a lazy programmer's waste of space. */
char aspace[ROWSIZE][ROWSIZE];
char dspace[ROWSIZE][ROWSIZE];

char rowtext[10*ROWSIZE];
char tmp[ROWSIZE];

char title[80];
char author[80];
char copyright[80];
char matrix[ROWSIZE*ROWSIZE*10];
char aclues[ROWSIZE*15*16];
char dclues[ROWSIZE*15*16];

void fgetaline(char *, FILE *);

void main(int argc, char *argv[])
{

	/* Open the puzzle file. */
	if (argc < 2) {
		printf("Usage: nytconv <puzzle file>\n");
		exit(1);
	}
	if (argc == 3 && argv[2] == "-s") {
		solve = 1;
	} else {
		solve = 0;
	}

	infile = fopen(argv[1], "rb");
	if (infile == NULL) {
		printf("Error opening %s.\n");
		exit(1);
	}

	/* From here on we're assuming a non-corrupt puzzle file.
		An enhancement later could be to check for this. */

	/* Read in the puzzle size, skipping the stuff 
		we don't understand. */
	fread(inbuf, 46, 1, infile);
	width =  (int)(inbuf[44]);
	height = (int)(inbuf[45]);
     
	/* Read in the solution, assuming(!) it is not scrambled.
		This may be a bad idea but we do it anyhow. */
	fread(inbuf, 6, 1, infile);  /* more basura */
	for (j=0; j<height; j++) fread(&aspace[j][0], width, 1, infile);

	/* Read in the diagram info separately.  It's safer to use
		this to deduce the numbering scheme. */
	for (j=0; j<height; j++) fread(&dspace[j][0], width, 1, infile);

	/* Now start to separate some of the text headers. */
	apos=dpos=0;
	fgetaline(title, infile);
	fgetaline(author, infile);
	fgetaline(copyright, infile);

	/* Now we are ready to produce the LaTeX stuff for the
		puzzle diagram.  Again we are assuming we have an unscrambled
		solution.  How can we know this?  Put on TODO list. */

	/* We write to standard out.  Lazy but effective. */

	printf("\\documentclass{article}\n");
	printf("\\usepackage{cwpuzzle}\n");
	printf("\\title{%s}\n", title);
	printf("\\author{%s}\n", author);
	printf("\\date{%s}\n", copyright);
	printf("\\voffset -1in\n");
	printf("\\hoffset -1in\n");
	printf("\\begin{document}\n");
	if (solve) {
		printf("\\PuzzleSolution\n");
	}
  
	ccount = 0;
	mcount = 0;

	for (j=0; j<height; j++) {
		strcpy(rowtext,"\0");
		
		for (k=0; k<width; k++) {
			/* Analyze position for across-number */
			/* Left edge non-black followed by non-black */
			anum = 0;
			if (  (k==0 && dspace[j][k]=='-' && dspace[j][k+1]=='-') ||
			/* Previous black - nonblack - nonblack */
			(  (k+1)<width && (k-1)>=0 && dspace[j][k]=='-' 
				&& dspace[j][k-1]=='.'
				&& dspace[j][k+1]=='-'  )
			){
				ccount++;   
				anum = ccount;
			}
			/* Analyze position for down-number */
			dnum = 0;
			/* Top row non-black followed by non-black */
			if (  (j==0 && dspace[j][k]=='-' && dspace[j+1][k]=='-') ||
			/* Black above - nonblack - nonblack below */
			(  (j-1)>=0 && (j+1)<height && dspace[j][k]=='-'
				&& dspace[j-1][k]=='.'
				&& dspace[j+1][k]=='-' )
			){
				/* Don't double number the same space */
				if (anum == 0) ccount++;
				dnum = ccount;
			}

			if(!solve) {
				/* Now, if necessary, pick up some clues and save them. */
				if (anum != 0) 	{
						fgetaline(tmp, infile);  
					sprintf(&aclues[apos], "\\Clue{%d}{}{%s}\\\\\n", 
						anum, tmp);
					apos = strlen(aclues);
				}
				if (dnum != 0)  {
						fgetaline(tmp, infile);  
					sprintf(&dclues[dpos], "\\Clue{%d}{}{%s}\\\\\n", 
						dnum, tmp);
					dpos = strlen(dclues);
				}
			}
			/* Concat the current info to the current row. */
			strcat(rowtext, "|");
			if (dspace[j][k] == '.') strcat(rowtext, "*");
			else {
				/* see if we need a grid number */
				if (dnum != 0) 	{
					sprintf(tmp, "[%d]", dnum);
					strcat(rowtext, tmp);
				}
				else if (anum != 0)  {
					sprintf(tmp, "[%d]", anum); 
					strcat(rowtext, tmp);
				}
				/* Put in solution letter */
				strncat(rowtext, &aspace[j][k], 1);
			}
        }
		/* End of the row.  Put in marker and output it. */
		strcat(rowtext, "|.");
		sprintf(&matrix[mcount], "%s\n", rowtext);
		mcount = strlen(matrix);
	}
   /* That's it for the puzzle. */

	if (!solve) {
		/* Now output the clue section */
		printf("\\begin{PuzzleClues}{\\textbf{Across}}\\\\\n");
		printf("%s", aclues);
		printf("\\end{PuzzleClues}\n");
		printf("\\begin{PuzzleClues}{\\textbf{Down}}\\\\\n");
		printf("%s", dclues);
		printf("\\end{PuzzleClues}\n");
		printf("\\newpage\n");
	}
	/* Now output the diagram */
	printf("\\maketitle\n");
	printf("\\begin{Puzzle}{%d}{%d}\n", width, height);
	printf("%s", matrix);
	printf("\\end{Puzzle}\n");

	printf("\\end{document}\n");

	exit(0);
}

void fgetaline(char *foo, FILE *infile) 
{
	unsigned char i;
	int oe;
	oe = 0;
	strcpy(foo, "\0");
	
	while (1) {
		fread(&i, 1, 1, infile);
		if (feof(infile) || (i=='\0')) break;
		if (i == 169) {
			strcat(foo, "\\copyright ");
		}
		else {
			if (i == '_' || i == '&' || i == '#' || i== '$') strcat(foo, "\\");
			if (i == '"') {
				if ( oe == 0) {
					oe = 1;
					strcat(foo, "``");
				}
				else {
					oe = 0;
					strcat(foo, "''");
				}
			}
			else strncat(foo, &i, 1);
		}
	}
}



