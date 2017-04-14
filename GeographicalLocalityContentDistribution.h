/*
 * ccnSim is a scalable chunk-level simulator for Content Centric
 * Networks (CCN), that we developed in the context of ANR Connect
 * (http://www.anr-connect.org/)
 *
 * People:
 *    Giuseppe Rossini (lead developer, mailto giuseppe.rossini@enst.fr)
 *    Raffaele Chiocchetti (developer, mailto raffaele.chiocchetti@gmail.com)
 *    Dario Rossi (occasional debugger, mailto dario.rossi@enst.fr)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * implemented by Alireza Montazeri (alireza.montazeri@usask.ca)
 */

#ifndef GEOGRAPHICALLOCALITYCONTENT_DISTRIBUTION_H
#define GEOGRAPHICALLOCALITYCONTENT_DISTRIBUTION_H

#include <omnetpp.h>
#include "ccnsim.h"
#include "content_distribution.h"
#include "zipf.h"
#include "zipf_sampled.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/tokenizer.hpp>

using namespace std;


/*
 * This module implements different distributions assigned to each single regions. Each region has a client moudle which will be generating its requests independent of other regions.
 * All the disitrubutions for each regions are presented in a file (e.g. request_patterns_file).
 * Each distribution is represented in one line of this file and has the folloiwng format:
 *      lambda alpha <the order of data items according to their popularity in that region>
 * This module is responsible to read the diffierent distributions for different regions from file
 * as well as the mapting between the clients to regions.
*/

class GeographicalLocalityContentDistribution : public content_distribution{

	public:

		/*
		 * the following structure stores details about the distribution for each region
		 */
		struct GeographicalLocalityInfoEntry{
			GeographicalLocalityInfoEntry(double _alpha, double _lambda, int _N):alpha(_alpha),lambda(_lambda),N(_N){}
			double alpha; // Zipf parameter
			double lambda; // request rate 
			int N; // number of data items in each region 
			vector<name_t> contents_order; // oreder of data items in a region

		};


		static vector<zipf_sampled*> zipfRegions; // list of distributions 
		int numOfRegions = 0; // number of regions
		long double totalContents = 0;
		

		vector<GeographicalLocalityInfoEntry> *regionInfo; // lis of regions and their corresponding distributions
		vector<bool> assignedRegionsToClients; // a distribution is assigned to a client; used only in casse of using random assignment for regions to clinets
		vector<name_t> all_contents;

		int get_region(int client_id); // retunrds the index of distribution associated to a region 


    protected:
		void initialize();
		void finish();

		void import_catalog_features(string request_patterns_file); // the distributions are read from request_patterns_file
		void import_client_to_region_map(string); // read the assigned distribution for each region from a text file
		void initialize_contents();

	private:
		int get_random_region();
		map<int,int> clientId_to_regionId_map;
		double total_request_rate; // the overall request rate from all the clients in the network; calculated from in the info in  request_patterns_file

};
#endif