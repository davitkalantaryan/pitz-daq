//
//
//
//


#include <stdlib.h>
#include <stdio.h>


int main(int argc, char* argv[])
{
    int nSystemReturn, nReturn = -1;

    if(argc<2){
        fprintf(stderr,"provide command to execute!\n");
        goto returnPoint;
    }

    nSystemReturn= system(argv[1]);
    printf("\n!!! systemReturn=%d\n",nSystemReturn);

    nReturn = 0;
returnPoint:
    return nReturn;
}
