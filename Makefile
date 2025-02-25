#General Purpose Makefile (Courtesy of Prateek Bhakta)

EXECS = assemble.exe
OBJS = project1.o

# Compiler and flags
CC = g++
CCFLAGS = -std=c++17 

# Build target
all: $(EXECS)

assemble.exe: $(OBJS)
	$(CC) $(CCFLAGS) -I . $^ -o $@

%.o: %.cpp *.h
	$(CC) $(CCFLAGS) -I . -c $< -o $@

clean:
	del /Q $(OBJS) $(EXECS) 2>nul
