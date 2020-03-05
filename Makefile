CXX ?= g++
sclus: Makefile Cluster.cpp Cluster.h ClusterContainer.cpp ClusterContainer.h Read.cpp Read.h ReadInfo.cpp ReadInfo.h Readpair.cpp Readpair.h clus.cpp
	$(CXX) -o sclus Cluster.cpp ClusterContainer.cpp Read.cpp ReadInfo.cpp Readpair.cpp clus.cpp
