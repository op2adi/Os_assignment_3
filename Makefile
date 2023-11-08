# this is makefile for MeMS

all: clean example 

example: example.c mems2.h
	gcc -o example example.c

clean:
	rm -rf example