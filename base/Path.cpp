#include <unistd.h>
#include <linux/limits.h>

#include "Path.h"

std::string GetCwd(){
    char path[PATH_MAX]={0};
    getcwd(path,PATH_MAX);
	return path;
}
