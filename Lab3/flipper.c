// Napisz program o nazwie "flipper", który na wejściu przyjmie ścieżkę do katalogu źródłowego
// oraz katalogu wynikowego. Program będzie przeszukiwał katalog źródłowy w poszukiwaniu plików tekstowych,
// otwierał je i odwracał kolejność znaków w każdej linii. Następnie zmodyfikowane pliki zostaną zapisane w katalogu wynikowym.
// Jako dane wejściowe wykorzystaj katalog z archiwum "art.tgz" dołączonego do tego ćwiczenia.

#include <stdio.h>
#include <dirent.h>
#include <regex.h>
#include <string.h>

int charcount(FILE *const fin)
{
    int c, count;

    count = 0;
    for (;;)
    {
        c = fgetc(fin);
        if (c == EOF || c == '\n')
            break;
        ++count;
    }

    return count;
}

void rev(char *s)
{

    // Initialize l and r pointers
    int l = 0;
    int r = strlen(s) - 1;
    char t;

    // Swap characters till l and r meet
    while (l < r)
    {

        // Swap characters
        t = s[l];
        s[l] = s[r];
        s[r] = t;

        // Move pointers towards each other
        l++;
        r--;
    }
}

int main(void)
{
    char src_dir[80];
    char dest_dir[80];

    struct dirent *de; // Pointer for directory entry
    printf("Enter a path to source directory");

    scanf("%s", src_dir);
    DIR *source_dirp = opendir(src_dir);

    if (source_dirp == NULL) // opendir returns NULL if couldn't open directory
    {
        printf("Could not open given directory");
        return 1;
    }
    printf("Enter a path to output directory");
    scanf("%s", dest_dir);
    DIR *destination_dirp = opendir(dest_dir);
    if (destination_dirp == NULL) // opendir returns NULL if couldn't open directory
    {
        printf("Could not open out directory");
        return 1;
    }

    regex_t regex;
    if (regcomp(&regex, "\\.txt$", REG_EXTENDED))
    {
        fprintf(stderr, "Could not compile regex\n");
        return 1;
    }
    FILE *source_fptr;
    FILE *out_fptr;
    char src_path[1024];
    char dest_path[1024];

    while ((de = readdir(source_dirp)) != NULL)
        if (!regexec(&regex, de->d_name, 0, NULL, 0))
        {
            snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, de->d_name);

            snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, de->d_name);

            printf("%s\n", src_path);
            source_fptr = fopen(src_path, "r");
            if (source_fptr == NULL)
            {
                perror("Błąd otwierania pliku");
                continue;
            }
            int line = charcount(source_fptr);
            char myString[line];
            out_fptr = fopen(dest_path, "w");
            while (fgets(myString, sizeof(myString), source_fptr) != NULL)
            {
                rev(myString);
                fputs(myString, out_fptr);
            }
            fclose(out_fptr);
            fclose(source_fptr);
        }

    regfree(&regex);
    closedir(source_dirp);
    closedir(destination_dirp);
    return 0;
}