# Summary
_simple_ls_ is an implementation of the UNIX utility ```ls -lR```. It resursively lists the contents of the current (or specified) directory in the long format. By default, the command lists all files, including the hidden ones.

# Usage

Use make to compile the program.

Example usage:

```
./simple_ls .
```

Example output:

```
.:
total 64
-rw-r--r--  1 isulimova staff    77 Oct 16 2017 Makefile
-rw-r--r--  1 isulimova staff   165 Oct 21 2017 README.md
-rwxr-xr-x  1 isulimova staff 14812 Oct 21 2017 simple_ls
-rw-r--r--  1 isulimova staff  6617 Oct 16 2017 simple_ls-lR.c
```


# Task List
- [ ] Add the capability for simple (short) listing format
- [ ] In the date field, list time instead of year for the files modified within the last 6 months
