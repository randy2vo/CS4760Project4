# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

# Executables
TARGETS = oss worker

# Object files
OSS_OBJS = oss.o
WORKER_OBJS = worker.o

# Default target
all: $(TARGETS)

# Build oss
oss: $(OSS_OBJS)
	$(CXX) $(CXXFLAGS) -o oss $(OSS_OBJS)

# Build worker
worker: $(WORKER_OBJS)
	$(CXX) $(CXXFLAGS) -o worker $(WORKER_OBJS)

# Suffix rule for compiling .cpp to .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

# Clean up build files
clean:
	rm -f *.o oss worker

# Optional: clean logs too
cleanall:
	rm -f *.o oss worker *.log

# Rebuild everything
rebuild: clean all
