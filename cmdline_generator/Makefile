CXX=g++
CXXFLAGS= -std=c++11 -I $(DPDK_DIR)/include
LDFLAGS=-L $(DPDK_DIR)/lib -ldpdk -ldl -latomic -lpthread -lrt -lm -ludev -lmicrohttpd -L /usr/local/lib/ -l :libjson_spirit.so

all: cmdline_generator

cmdline_generator: ivshmem_cmdline_generator.o rest_server.o cmdline_generator.o logger.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS)

clean:
	rm *.o cmdline_generator
