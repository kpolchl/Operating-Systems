#include <stdio.h>
#include <time.h>

int main()
{
    double time_spent = 0.0;
    clock_t begin = clock();
    FILE *fptr;

    // Open a file in read mode
    fptr = fopen("lorem.txt", "r");

    // Store the content of the file
    char myString[1024];

    fseek(fptr,100,1);
    // Read the content and print it
    while (fgets(myString, 1024, fptr))
    {
        printf("%s", myString);
    }

    // Close the file
    fclose(fptr);

    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("The elapsed time is %f seconds", time_spent);
    return 0;
}