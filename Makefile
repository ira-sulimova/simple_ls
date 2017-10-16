CC=gcc

simple_ls: 
	$(CC) -o simple_ls simple_ls-lR.c 

clean:
	rm simple_ls