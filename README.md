These two classes introduce geographical locality for clients' requests.
GeographicalLocalityContentDistribution class which inherets content_distribution calss is repsonsible for the followings: 
  1. Mapping each client to a region. Each region has its own distribution.
  2. Providing the details of information, including lambda, alpha  and the order of data items according to their popularity in that region, for each region. 
  
client_GeographicalLocality class which inherets client class generates its requests according to the client to which its assgined.

Copy GeographicalLocalityContentDistribution.h and client_GeographicalLocality.h to ccnsim/include
Copy client_GeographicalLocality.cc to ccnsim/src/clients
Copy GeographicalLocalityContentDistribution.cc to ccnsim/src/content
Copy GeographicalLocalityContentDistribution.ned to ccnsim/moduels/contnet
Copy omnet.ini to ccnsim
