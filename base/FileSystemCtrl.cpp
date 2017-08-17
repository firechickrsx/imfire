#include "FileSystemCtrl.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>


 int FileSystemCtrl::CreateMutiDir(const char* sPathName)
{
    char DirName[512];
    strcpy(DirName,sPathName);
    int i,len = strlen(DirName);
    if(DirName[len-1]!='/')
        strcat(DirName,"/");
    len = strlen(DirName);
    for(i=1; i<len; i++)
    {
        if(DirName[i]=='/')
        {
            DirName[i] = 0;
            if(access(DirName,F_OK) != 0)
            {
                if(mkdir(DirName,0777) == -1)
                {
                    printf("mkdir error");
                    return -1;
                }
            }
            DirName[i] = '/';
        }
    }
    return 0;

}
