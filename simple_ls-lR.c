#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <time.h>

#define DIRCAP 10000
#define MAXPATHLEN 500

typedef struct file_information {
    char name[200];
    char path[MAXPATHLEN];
    struct stat st;
} file_info;

file_info entries[DIRCAP] = {0};
int entry_count = 0;
char directories[DIRCAP][MAXPATHLEN] = {{0}};
int dir_count = 0;
int curr_dir = 0;

// ls printing table columns:
// [0] links, [1] user, [2] user group, [3] size, [4] time/year, [5] day
int column_indent[6] = {0};


void my_ls(char * path);

void error(char * reason)
{
    perror(reason);
    exit(1);
}


void print_permissions( struct stat st )
{
    printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
    printf( (st.st_mode & S_IRUSR) ? "r" : "-");
    printf( (st.st_mode & S_IWUSR) ? "w" : "-");
    printf( (st.st_mode & S_IXUSR) ? "x" : "-");
    printf( (st.st_mode & S_IRGRP) ? "r" : "-");
    printf( (st.st_mode & S_IWGRP) ? "w" : "-");
    printf( (st.st_mode & S_IXGRP) ? "x" : "-");
    printf( (st.st_mode & S_IROTH) ? "r" : "-");
    printf( (st.st_mode & S_IWOTH) ? "w" : "-");
    printf( (st.st_mode & S_IXOTH) ? "x" : "-");
    putc(' ', stdout);
}


void get_time( char * buffer, time_t t, char *day )
{
    struct tm time;
    localtime_r(&t, &time);
    
    if (time.tm_year == 2015-1900)
        sprintf( buffer, "%02d:%02d", time.tm_hour, time.tm_min);
    else
        sprintf( buffer, "%-d", time.tm_year + 1900);

    sprintf(day, "%d", time.tm_mday);
}


void print_time( time_t t )
{
    char d[10];
    struct tm time;
    localtime_r(&t, &time);
    char * months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char year[10] = {0};
    get_time( year, t, d);
    printf("%s %*d %*s ", months[time.tm_mon], column_indent[5], time.tm_mday, column_indent[4], year);
}


void ls_file( char * path, struct stat st, char * filename)
{
    print_permissions(st);
    char buff[100] = {0};
    
    printf("%*d ", column_indent[0], st.st_nlink);
    printf("%*s ", column_indent[1], getpwuid(st.st_uid)->pw_name);
    printf("%*s ", column_indent[2], getgrgid(st.st_gid)->gr_name);
    sprintf(buff, "%lld", st.st_size);
    printf("%*s ", column_indent[3], buff);
    print_time(st.st_mtime);
    printf("%s", filename);
    putc('\n', stdout);
}


void set_col_indent( struct stat st)
{
    char links[50] = {0};
    char size[50] = {0};
    char year[10] = {0};
    char day[5] = {0};
    
    sprintf(links, "%d", st.st_nlink);
    sprintf(size, "%lld", st.st_size);
    get_time(year, st.st_mtime, day);
    
    if ( strlen(links) > column_indent[0] ) column_indent[0] = strlen(links);
    if ( strlen(getpwuid(st.st_uid)->pw_name) > column_indent[1] ) column_indent[1] = strlen(getpwuid(st.st_uid)->pw_name);
    if ( strlen(getgrgid(st.st_gid)->gr_name) > column_indent[2] ) column_indent[2] = strlen(getgrgid(st.st_gid)->gr_name);
    if ( strlen(size) > column_indent[3] ) column_indent[3] = strlen(size);
    if ( strlen(year) > column_indent[4] ) column_indent[4] = strlen(year);
    if ( strlen(day) > column_indent[5] ) column_indent[5] = strlen(day);
}


void add_file(char * path, struct stat st, char * name)
{
    set_col_indent(st);
    strcpy(entries[entry_count].name, name);
    strcpy(entries[entry_count].path, path);
    entries[entry_count].st = st;
    ++entry_count;
}


void add_dir(char * path)
{
    strcpy(directories[dir_count++], path);
}


void pop_dir(char * path)
{
    strcpy(path, directories[--dir_count]);
}


void strip_chars( char * new_str, const char * old_str)
{
    int c = 0, i = 0;
    const char * p;
    for (p = old_str; i < strlen(old_str); ++p, ++i)
    {
        if ((*p) != '.' && (*p) != '-' && (*p) != '_' && (*p) != ',')
            new_str[c++] = (*p);
    }
}


int compare_entry(const void * p1, const void * p2)
{
    const file_info * x = p1;
    const file_info * y = p2;
    
    char name1[100] = {0};
    char name2[100] = {0};
    
    strip_chars(name1, x->name);
    
    strip_chars(name2, y->name);
    return strcasecmp(name1, name2);
}


void get_dirs()
{
    int i;
    for (i=entry_count-1; i >= 0; --i)
    {
        if ( entries[i].st.st_mode & S_IFDIR )
            add_dir(entries[i].path);
    }
}


void print_entries( int entry_count )
{
    int i;
    for (i = 0; i < entry_count; ++i)
    {
        ls_file(entries[i].path, entries[i].st, entries[i].name);
    }
}


int add_all_entries_from(DIR * directory, char * path)
{
    struct dirent * entry;
    char new_path[MAXPATHLEN];
    struct stat st_entry;
    int block_size = 0;


    entry_count = 0;
    while ( (entry = readdir(directory)) != NULL)
    {
        if ( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            sprintf(new_path, "%s/%s", path, entry->d_name);
            stat(new_path, &st_entry);
            block_size += st_entry.st_blocks;
            add_file(new_path, st_entry, strrchr(new_path, '/')+1);
        }
    }

    return block_size;
}


int ls_directory( char * path, int curr_dir)
{
    memset(column_indent, 0, 5 * sizeof(int));
    DIR * directory;
    struct dirent * entry;
    char new_path[MAXPATHLEN];
    struct stat st_dir, st_entry;
    int block_size = 0;
    

    if ( stat(path, &st_dir) == -1 )
    {
        error(path);
    }
    if ( (directory = opendir(path)) == NULL )
    {
        fprintf(stderr, "Can't open directory %s\n", path);
        return block_size;
    }

    
    block_size = add_all_entries_from(directory, path);

    printf("%s:", path);
    printf("\ntotal %d\n", block_size);
    
    qsort(entries, entry_count, sizeof(file_info), compare_entry);
    get_dirs();
    print_entries( entry_count );

    while (curr_dir != dir_count && dir_count != 0)
    {
        char temp_path[MAXPATHLEN] = {0};
        putc('\n', stdout);
        pop_dir(temp_path);
        ls_directory(temp_path, dir_count + 1);
    }

    return block_size;
}


void my_ls(char * path)
{
    struct stat st;
    if ( stat(path, &st) != -1 )
    {
        if ( st.st_mode & S_IFDIR )
        {
            if (path[strlen(path)-1] == '/')
                path[strlen(path)-1] = '\0';
            ls_directory(path, 0);
        }
        else if ( st.st_mode & S_IFREG )
        {
            ls_file(path, st, strrchr(path, '/')+1);
        }
    }
    else
    {
        error("Stat error");
    }
}


int main(int argc, char const *argv[])
{
    argc == 1? my_ls(".") : my_ls(argv[1]);
    
    return 0;
}