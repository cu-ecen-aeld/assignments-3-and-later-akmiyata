#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char *argv[])
{
    char *writefile;
    char *writestring;
    FILE * fw;

    openlog(NULL, 0, LOG_USER);

    // Check argument input
    if(argc == 3)
    	{
	//Initialize write variables; log info    
        writefile = argv[1];
        writestring = argv[2];
        syslog(LOG_DEBUG, "Writing %s to %s", writefile, writestring);

        //Write file & close
        fw=fopen(writefile, "w+");
	fputs(writestring, fw);
	fclose(fw);
	
	//If file fails to write:
	if (!fw)
		{
		printf("Unable to create file.\n");
		syslog(LOG_ERR, "Unable to create file.");
		return 1;	
    		};
	}

    else //Too few arguments
    	{
        syslog(LOG_ERR, "Missing arguments");
        printf("%s", "Missing arguments; two are required.\n");
        return 1;
    	};
    return (0);
}
