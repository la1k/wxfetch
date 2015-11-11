#include <stdio.h>
#include <math.h>

main()
{
    int x, y;
    double d, c;

    for (y = -2; y < 3; y++) {
	for (x = -2; x < 3; x++) {

	    d = sqrt((double) (x * x + y * y)) / 4;

	    c = sin(2.0 * M_PI * d) * sin(M_PI * d) / (2.0 * M_PI * M_PI *
						       d * d);

	    printf("%5g ", c);
	}
	printf("\n");
}}
