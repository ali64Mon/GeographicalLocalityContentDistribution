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


#include "ccnsim.h"
#include "GeographicalLocalityContentDistribution.h"
#include "content_distribution.h"
#include <error_handling.h>
#include <core_layer.h>
#include <fstream>
#include <algorithm>

Register_Class(GeographicalLocalityContentDistribution);

vector<zipf_sampled*> GeographicalLocalityContentDistribution::zipfRegions;


void GeographicalLocalityContentDistribution::initialize()
{
	cout << "Initialize GEOGRAPHICAL LOCALITY content distribution...\tTime:\t" << SimTime() << "\n";

	regionInfo = new vector<GeographicalLocalityInfoEntry>();
	
	import_catalog_features(par("request_patterns_file").stdstringValue());
	import_client_to_region_map(par("client_to_region_map_file").stdstringValue());
	
	
	initialize_contents();
	
	content_distribution::initialize();

}



void GeographicalLocalityContentDistribution::import_catalog_features(string request_patterns_file){


	/*
     * the request_patterns_file format
     * each region is represented in one line as follows:
	 *			lambda alpha <the order of data items according to their popularity in that region>
	*/
	
	ifstream f;
	
	string file_path;
	file_path = request_patterns_file;
	cout<<"--------------------------------------"<<endl;
	cout<<"reading files from:"<<file_path<<endl;
	cout<<"--------------------------------------"<<endl;
	f.open(file_path.c_str());

	if(!f){
		error("The file representing the local requests patterns is not provided!!!");
	}

	numOfRegions = 0;
	total_request_rate = 0;
	for ( string line; getline(f,line);){

		

		GeographicalLocalityInfoEntry* tempInfoEntry = new GeographicalLocalityInfoEntry (0,0,0);

		char *cstr = new char[line.length() + 1];
		strcpy(cstr, line.c_str());
		char * pch;
		pch = strtok(cstr, " \t\n\r");

		int term_count = 0;
		while (pch != NULL) {

			if (pch[0] != '\0') {

				if (term_count == 0){
					tempInfoEntry->lambda = atof(pch);
					total_request_rate += atof(pch);
					cout<<"lambda: "<<atof(pch)<<",";
					
				}
				else if (term_count == 1){
					tempInfoEntry->alpha = atof(pch);
					cout<<"alpha: "<<atof(pch)<<endl;
				
				}
				else{
					name_t content_n = (name_t) atoi(pch);
					tempInfoEntry->contents_order.push_back(content_n);
					// add the content to all_contents if not already added
					vector<name_t>::iterator temp_it = find(all_contents.begin(),all_contents.end(),content_n);
					if (temp_it == all_contents.end() )
						all_contents.push_back(content_n);
					
				}
				term_count++;

			}
			pch = strtok(NULL, " \t\n\r");
			
		}

		tempInfoEntry->N = term_count-2;		
		regionInfo->push_back(*tempInfoEntry);
		numOfRegions++;
	}

	f.close();


	cout<<"number of regions read from the request pattern file: "<<numOfRegions<<endl;
	if (getAncestorPar("num_clients").longValue() != numOfRegions )
			error("The number of clients provided in the request pattern file and the omnetpp.ini do not match!!!");


	totalContents = all_contents.size();
	assignedRegionsToClients.resize(numOfRegions,false);

	
	cout<<"reading file done"<<endl;

}


void GeographicalLocalityContentDistribution::import_client_to_region_map(string client_to_region_map_file){

	/*
     * the client_to_region_map_file format
     * each map is represented in one line as follows:
	 *		c r
	 *		c is the cient id and r is the region (distribution) id based on which c produces requets.
	*/

	if (client_to_region_map_file == "")
		return;


	ifstream f;
	//string line;
	string file_path;
	file_path =  client_to_region_map_file;
	cout<<"--------------------------------------"<<endl;
	cout<<"reading files from:"<<file_path<<endl;
	cout<<"--------------------------------------"<<endl;
	f.open(file_path.c_str());

	if(!f){
		error("The file representing the client to region map is not provided!!!");
	}
	int map_num = 0;
	for ( string line; getline(f,line);){

		char *cstr = new char[line.length() + 1];
		strcpy(cstr, line.c_str());
		char * pch;
		int client_id,region_id;
		pch = strtok(cstr, " \t\n\r");
		client_id = atoi(pch);
		pch = strtok(NULL, " \t\n\r");
		region_id = atoi(pch);
		clientId_to_regionId_map[client_id] = region_id;

		map_num++;

	}

	f.close();

	if (getAncestorPar("num_clients").longValue() != map_num )
			error("The number of maps in client_to_region_map_file does not match the  num_clients!!!");
	cout<<"reading file done"<<endl;

}

void GeographicalLocalityContentDistribution::initialize_contents(){

	// initializing the distribution for each region
	for ( int r = 0; r < numOfRegions; r ++){

		double alpha = regionInfo->operator [](r).alpha;		// Zipf's exponent for the catalog in region r.
		long cardF = regionInfo->operator [](r).N; 	// Total number of files within region r.
		double lambda = regionInfo->operator [](r).lambda;

		zipf_sampled* zipf = new zipf_sampled(cardF,alpha,lambda,1);  
		zipf->zipf_sampled_initialize();
		zipfRegions.push_back(zipf);
	}
}

/*
 * This functions assignes distributions to clients randomly if the file depicting this map in not provided in .ini file
*/
int GeographicalLocalityContentDistribution::get_random_region(){
	int guess;
	
	do{
	
		guess = rand()%numOfRegions;

	}
	while (assignedRegionsToClients.operator[](guess));

	assignedRegionsToClients.operator[](guess) = true;
	return guess;
}


/*
 * Retunrns the region (distribution) assigned to a client
*/
int GeographicalLocalityContentDistribution::get_region(int client_id){

	if (par("client_to_region_map_file").stdstringValue() == "")
		return get_random_region();
	return clientId_to_regionId_map[client_id];
}


void GeographicalLocalityContentDistribution::finish()
{
	delete regionInfo;
	for (uint32_t i=0; i<zipfRegions.size(); i++)
		delete zipfRegions[i];
}
