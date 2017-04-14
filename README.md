Thee following two classes introduce geographical locality for clients' requests.

- GeographicalLocalityContentDistribution class which inherets content_distribution calss is repsonsible for the followings: 
  1. Mapping each client to a region. Each region has its own distribution.
  2. Providing the details of information, including lambda, alpha  and the order of data items according to their popularity in that region, for each region. 
  
- client_GeographicalLocality class which inherets client class generates its requests according to the client to which its assgined.

Since different regions have differnt lambda, we can also dedicate different cache sizes to the nodes in the network.
In this regard, we modify basic_cache to accept differnt caches sizes for nodes. This is implemented as a map between a node and its corresponding size provided a file.


Copy GeographicalLocalityContentDistribution.h, client_GeographicalLocality.h and basic_cache.h to ccnsim/include/ \
Copy client_GeographicalLocality.cc to ccnsim/src/clients/ \
Copy GeographicalLocalityContentDistribution.cc to ccnsim/src/content/ \
Copy GeographicalLocalityContentDistribution.ned to ccnsim/moduels/contnet/ \
Copy omnetpp.ini to ccnsim/ \
Replace basic_cache.cc with its original version in ccnsim/src/node/cache/ \
Replace Makefile with its original in ccnsim/
Replace the following  with the one in ccnsim/modles/node/cache/caches.ned 


	simple base_cache like cache
	{
	    parameters:
		@display("i=block/buffer2;is=l");

		string DS = default("lce");
		int C = default (100);
	  string cache_budget_file = default("");
	    gates:
		inout cache_port;
	}

Note: these two classes are compatible with ccnsim3.

Make the project, configure the .ini file and run it.
