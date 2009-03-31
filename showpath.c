#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **entries;
size_t num,max;

void grow(void)
{
	if(num >= max)
	{
		void *t=realloc(entries,2*max*sizeof *entries);
		if(!t)
		{
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		entries=t;
		max*=2;
	}
}

/*strdup is *nix, and glibc with gcc -ansi -pedantic doesn't give a
    declaration for it.
*/
char *strdup(const char *s);

int add_entry(const char *new)
{
	size_t i;

	for(i=0;i<num;i++)
		if(strcmp(entries[i],new)==0)
			return 0;

	grow();
	entries[num]=strdup(new);
	if(!entries[num])
	{
		perror("strdup");
		exit(EXIT_FAILURE);
	}
	num++;
	return 0;
}

int add_path(void)
{
	const char *pp=getenv("PATH");
	char *p,*q;
	if(!pp)
	{
		perror("getenv: can't get $PATH");
		return -1;
	}
	/*getenv doesn't necessarily give us a string we can modify*/
	p=strdup(pp);
	if(!p)
	{
		perror("strdup");
		fputs("Failed getting $PATH!\n",stderr);
		return -1;
	}
	q=strtok(p,":");
	while(q)
	{
		int ret=add_entry(q);
		if(ret)
			return ret;
		q=strtok(NULL,":");
	}

	return 0;
}

int main(int argc,char **argv)
{
	int i;

	if(argc<2)
	{
		printf("Usage: %s path-entry ...\n",argv[0]);
		printf("  Outputs a path containing all path-entries on the command line, in\n");
		printf("    the order given, with duplicates removed.\n");
		printf("  The magic path-entry '%%current' expands to the current value of $PATH.\n");
		return 0;
	}

	entries=malloc(sizeof *entries);
	if(!entries)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	max=1;

	for(i=1;i<argc;i++)
	{
		int ret;
		if(strcmp(argv[i],"%current")==0)
			ret=add_path();
		else
			ret=add_entry(argv[i]);
		if(ret)
		{
			fprintf(stderr,"%s: Adding path entry '%s' failed!\n",argv[0],argv[i]);
			return EXIT_FAILURE;
		}
	}

	if(num)
	{
		size_t idx;
		printf("%s",entries[0]);
		for(idx=1;idx<num;idx++)
			printf(":%s",entries[idx]);
		printf("\n");
	}

	return 0;
}
