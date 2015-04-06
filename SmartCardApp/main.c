#include "header.h"
int main(int argc,char *argv[])
{
	FILE *fp;
	char ch;
	char *arg2;

	if(argc==2)
	{
		if(strcmp(argv[1],"secret")==0)
		{
			secret();
		}
		else
		{
		fprintf(stderr,"{malformed command}\n");
		exit(1);
		}
	}
	else if(argc==3)
	{
		if(strcmp(argv[1],"verify")==0)
		{
	        	if(strncmp(argv[2],"-so=",4)==0 && strlen(argv[2])>3)
                        {
                                if((fp=fopen("create_stdin","w"))==NULL)
                                {
                                fprintf(stderr,"{input file %s does not exist}\n",argv[2]);
                                exit(1);
                                }
                                while(fread(&ch,1,1,stdin))
                                {
                                fwrite(&ch,1,1,fp);
                                }
                                fclose(fp);
                                arg2=argv[2]+4;
                                verify(arg2,"create_stdin");
                        }
                        else
                        {
                        fprintf(stderr,"{malformed command}\n");
                        exit(1);
			}

		}
		else
		{
			fprintf(stderr,"{malformed command}\n");
		}
	}
	else if(argc==4)
	{
		if(strcmp(argv[1],"verify")==0)
		{
			if(strncmp(argv[2],"-so=",4)==0 && strlen(argv[2])>3)
			{
				arg2=argv[2]+4;
				verify(arg2,argv[3]);
			}
			else
			{
			fprintf(stderr,"{malformed command}\n");
			exit(1);
			}
		}
		else if(strcmp(argv[1],"sign")==0)
		{
			if(strncmp(argv[2],"-so=",4)==0 && strlen(argv[2])>3)
			{
			arg2=argv[2]+4;
			sign(arg2,argv[3]);
			}
			else
			{
			fprintf(stderr,"{malformed command}\n");
			exit(1);
			}
		}
		else if(strcmp(argv[1],"enc")==0)
		{
			if(strncmp(argv[2],"-so=",4)==0 && strlen(argv[2])>3)
			{
			arg2=argv[2]+4;
			enc(arg2,argv[3]);
			}
			else
			{
			fprintf(stderr,"{malformed command}\n");
			exit(1);
			}
		}
	        else if(strcmp(argv[1],"dec")==0)
                {
                        if(strncmp(argv[2],"-so=",4)==0 && strlen(argv[2])>3)
                        {
                        arg2=argv[2]+4;
                        dec(arg2,argv[3]);
                        }
                        else
                        {
                        fprintf(stderr,"{malformed command}\n");
                        exit(1);
                        }
                }
		else if(strcmp(argv[1],"edit")==0)
                {
                        if(strncmp(argv[2],"-so=",4)==0 && strlen(argv[2])>3)
                        {
			printf("I HAVE NOT IMPLEMENTED THIS PROGRAM, SO PLEASE DO NOT TEST THIS PROGRAM. THANK YOU.\n");
			exit(1);
                        arg2=argv[2]+4;
                        edit(arg2,argv[3]);
                        }
                        else
                        {
                        fprintf(stderr,"{malformed command}\n");
                        exit(1);
                        }
                }
		else 
		{
		fprintf(stderr,"{malformed command}\n");
		exit(1);
		}
	}
	else
	{
		fprintf(stderr,"{malformed command}\n");
		exit(1);
	}
	return 1;
}

