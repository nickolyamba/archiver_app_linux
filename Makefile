# Name: Nikolay Goncharenko
# Email: goncharn@onid.oregonstate.edu
# Class: CS344-400
# Assignment: homework#3
#######################################

# gcc -g -Wall -o  stat stat.c

CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG) -Wall
C99 = -std=c99
HEAD = file_perms.h
PROGS = sig_demo myar
RM = rm -f ar12345.ar myar12345.ar
CREATE_AR = ar q ar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt


all: $(PROGS)

sig_demo: sig_demo.o
	$(CC) $(CFLAGS) $(C99) -o sig_demo sig_demo.o

sig_demo.o: sig_demo.c
	$(CC) $(CFLAGS) $(C99) -c sig_demo.c
    

myar: myar.o file_perms.o
	$(CC) $(CFLAGS) $(C99) myar.o file_perms.o -o myar

myar.o: myar.c
	$(CC) $(CFLAGS) $(C99) -c myar.c
    
file_perms.o: file_perms.c
	$(CC) $(CFLAGS) $(C99) -c file_perms.c

    
clean:
	rm -f $(PROGS) *.o *~ *#    

##############################################
####################testq#####################	
testq12345: 
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
	myar -q myar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
	diff ar12345.ar myar12345.ar

testq135:
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 1-s.txt 3-s.txt 5-s.txt
	myar -q myar12345.ar 1-s.txt 3-s.txt 5-s.txt
	diff ar12345.ar myar12345.ar

testq24:
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 2-s.txt 4-s.txt
	myar -q myar12345.ar 2-s.txt 4-s.txt
	diff ar12345.ar myar12345.ar

testq: testq12345 testq135 testq24

##############################################
####################testt#####################
testt12345:
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
	ar t ar12345.ar > ar-ctoc.txt 
	myar -t ar12345.ar > myar-ctoc.txt
	diff ar-ctoc.txt myar-ctoc.txt
	
testt135:
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 1-s.txt 3-s.txt 5-s.txt
	ar t ar12345.ar > ar-ctoc.txt 
	myar -t ar12345.ar > myar-ctoc.txt
	diff ar-ctoc.txt myar-ctoc.txt

testt24:
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 2-s.txt 4-s.txt
	ar t ar12345.ar > ar-ctoc.txt 
	myar -t ar12345.ar > myar-ctoc.txt
	diff ar-ctoc.txt myar-ctoc.txt
	
testt: testt12345 testt135 testt24

##############################################
####################testv#####################

testv12345:
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
	ar tv ar12345.ar > ar-vtoc.txt 
	myar -v ar12345.ar > myar-vtoc.txt
	diff ar-vtoc.txt myar-vtoc.txt
	
testv135:
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 1-s.txt 3-s.txt 5-s.txt
	ar tv ar12345.ar > ar-vtoc.txt 
	myar -v ar12345.ar > myar-vtoc.txt
	diff ar-vtoc.txt myar-vtoc.txt

testv24:
	rm -f ar12345.ar myar12345.ar
	ar q ar12345.ar 2-s.txt 4-s.txt
	ar tv ar12345.ar > ar-vtoc.txt 
	myar -v ar12345.ar > myar-vtoc.txt
	diff ar-vtoc.txt myar-vtoc.txt

testv: testv12345 testv135 testv24

tests: testq testt testv