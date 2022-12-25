#include "set/set_coarse.hpp"
#include "set/set_fine.hpp"
#include "tester.hpp"

#include <iostream>
#include <fstream>

struct CfgParams {
	int nThreadsWriters;
	int sizeWriters;
	
	int nThreadsReaders;
	int sizeReaders;

	int numOfTests;
};

void ReadConfig(std::string path, CfgParams& params) {
	std::ifstream fin(path);
	
	fin >> params.nThreadsWriters >> params.sizeWriters;
	fin >> params.nThreadsReaders >> params.sizeReaders;
	fin >> params.numOfTests;

	fin.close();
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Enter path to config file as argument\n";
		return -1;
	}

	CfgParams params;
	ReadConfig(argv[1], params);

	std::cout << "Coarse-Grained:\n";
	Tester<SetCoarseSync<int>>::WritersFuncTest(params.nThreadsWriters, params.sizeWriters);
	Tester<SetCoarseSync<int>>::ReadersFuncTest(params.nThreadsReaders, params.sizeReaders);
	Tester<SetCoarseSync<int>>::GeneralFuncTest(params.sizeReaders, params.sizeWriters);
	Tester<SetCoarseSync<int>>::WritersPerfTest(params.nThreadsWriters, params.sizeWriters, params.numOfTests);
	Tester<SetCoarseSync<int>>::ReadersPerfTest(params.nThreadsWriters, params.sizeReaders, params.numOfTests);
	Tester<SetCoarseSync<int>>::GeneralPerfTest(params.sizeReaders, params.sizeWriters, params.numOfTests);

	std::cout << "\nFine-Grained:\n";
	Tester<SetFineSync<int>>::WritersFuncTest(params.nThreadsWriters, params.sizeWriters);
	Tester<SetFineSync<int>>::ReadersFuncTest(params.nThreadsReaders, params.sizeReaders);
	Tester<SetFineSync<int>>::GeneralFuncTest(params.sizeReaders, params.sizeWriters);
	Tester<SetFineSync<int>>::WritersPerfTest(params.nThreadsWriters, params.sizeWriters, params.numOfTests);
	Tester<SetFineSync<int>>::ReadersPerfTest(params.nThreadsWriters, params.sizeReaders, params.numOfTests);
	Tester<SetFineSync<int>>::GeneralPerfTest(params.sizeReaders, params.sizeWriters, params.numOfTests);
	
	return 0;
}
