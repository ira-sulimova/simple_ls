CC=gcc

simple_ls: simple_ls-lR.c 
	$(CC) -o simple_ls simple_ls-lR.c 

clean:
	rm simple_ls