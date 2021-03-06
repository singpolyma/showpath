#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>	/*for getopt*/

/* ugly globals */
char **entries = NULL;
size_t num = 0;
size_t max = 0;

/* stores argv[0] for use in output */
static char *myname;

/* Takes a char**, the number of elements in the buffer,
 * and the current buffer maxsize, and grows if necessary.
 */
char **grow(char **entries, size_t num, size_t *max)
{
	if(num >= *max)
	{
		void *t;
		*max = 2*(*max+1);
		t=realloc(entries,*max*sizeof *entries);
		if(!t)
		{
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		entries=t;
	}
	return entries;
}

/* Appends new to the global list of entries if it is 
 * not already in the list.
 * Grows the buffer if necessary.
 */
int add_entry(const char *new)
{
	size_t i;

	for(i=0;i<num;i++)
		if(strcmp(entries[i],new)==0)
			return 0;

	entries = grow(entries, num, &max);
	entries[num]=strdup(new);
	if(!entries[num])
	{
		perror("strdup");
		exit(EXIT_FAILURE);
	}
	num++;
	return 0;
}

/* Parse values out of a string and add them to entries.
 */
int add_list(const char *list,char sep)
{
	char *p=strdup(list);
	char *q;
	char seperator[2];

	seperator[0]=sep;
	seperator[1]='\0';

	if(!p)
	{
		perror("strdup");
		return -1;
	}
	q=strtok(p,seperator);
	while(q)
	{
		int ret=add_entry(q);
		if(ret)
			return ret;
		q=strtok(NULL,seperator);
	}

	free(p);

	return 0;
}

/* Look up an environment variable and add its contents to
 * entries.
 */
int add_from_env(char* env_var, char sep)
{
	const char *p=getenv(env_var);

	if(!p)
	{
		/*This isn't actually an error, it just means we have
		    an empty expansion for %current.
		  So silently continue with anything else we want to throw in.
		*/
		return 0;
	}

	return add_list(p,sep);
}

/* Set the global envname based on predefined types.
 * Used for -t switch.
 */
int set_type(const char *type, char **envname)
{
	const char *types[]={"exec","man",NULL};
	char *envs[]={"PATH","MANPATH",NULL};
	size_t i = -1;
	while(types[++i])
	{
		if(strcmp(types[i],type)==0)
		{
			*envname=envs[i];
			return 0;
		}
	}

	return 1;
}

void shortusage(void)
{
	printf("Usage: %s [-t type | -v env_name] [-s separator ] path-entry ...\n",myname);
}

void usage(void)
{
	shortusage();
	printf("  Outputs a path containing all path-entries on the command line, in\n");
	printf("    the order given, with duplicates removed.\n");
	printf("  Known types are 'exec' (for $PATH) and 'man' (for $MANPATH).\n");
	printf("  The magic path-entry '%%current' expands to the current value from\n");
	printf("    the environment.\n");
}

int main(int argc,char **argv)
{
	int i;
	int opt;
	int have_type=0;
	char *envname="PATH";
	char sep = ':';

	myname=argv[0];

	while((opt=getopt(argc,argv,":t:v:s:"))!=-1)
	{
		switch(opt)
		{
		case 't':
			if(have_type)
			{
				fprintf(stderr,"%s: Only one '-t' or '-v' allowed!\n",myname);
				shortusage();
				return EXIT_FAILURE;
			}
			if(set_type(optarg, &envname))
			{
				fprintf(stderr,"%s: Unknown type '%s'!\n",myname,optarg);
				shortusage();
				return EXIT_FAILURE;
			}
			have_type=1;
			break;
		case 'v':
			if(have_type)
			{
				fprintf(stderr,"%s: Only one '-t' or '-v' allowed!\n",myname);
				shortusage();
				return EXIT_FAILURE;
			}
			envname=optarg;
			have_type=1;
			break;
		case 's':
			if(optarg[1])
			{
				fprintf(stderr,"%s: Separator must be a single character!\n",myname);
				shortusage();
				return EXIT_FAILURE;
			}
			sep = optarg[0];
			break;
		case ':':
			fprintf(stderr,"%s: Option '%c' requires an argument\n",myname,optopt);
			shortusage();
			return EXIT_FAILURE;
		case '?':
			fprintf(stderr,"%s: Unrecognized option: %c\n",myname,optopt);
			shortusage();
			return EXIT_FAILURE;
		default:
			usage();
			return EXIT_FAILURE;
		}
	}

	/* These lines allow us to treat argc and argv as though 
	 * any switches were not there
	 */
	argc-=optind;
	argv+=optind;

	if(argc<1)
	{
		usage();
		return 0;
	}

	for(i=0;i<argc;i++)
	{
		int ret;
		if(strcmp(argv[i],"%current")==0)
			ret=add_from_env(envname, sep);
		else
			ret=add_list(argv[i],sep);
		if(ret)
		{
			/*XXX This will get ugly if we have a long path
			    on the command line.  Is there a better way
			    to handle that?   --DV
			*/
			fprintf(stderr,"%s: Adding path entry '%s' failed!\n",argv[0],argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	if(num)
	{
		size_t idx;
		printf("%s",entries[0]);
		for(idx=1;idx<num;idx++)
			printf("%c%s",sep,entries[idx]);
		printf("\n");
	}

	return 0;
}
