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


#include "content_distribution.h"

#include "statistics.h"

#include "ccn_interest.h"
#include "ccn_data.h"

#include "ccnsim.h"
#include "client_GeographicalLocality.h"

#include "error_handling.h"
#include <random>

Register_Class (client_GeographicalLocality);
GeographicalLocalityContentDistribution* client_GeographicalLocality::glPointer;

/*
 * The region this client is assigned to is determined during initialization phase as follows
 */
void client_GeographicalLocality::initialize(int stage)
{
	if(stage == 1)		// Multi-stage initialization (we need the GeographicalLocality pointer to be initialized)
	{
		int num_clients = getAncestorPar("num_clients");
		active = false;
		if (find(content_distribution::clients, content_distribution::clients + num_clients ,getNodeIndex()) != content_distribution::clients + num_clients)
		{
			// Initialize the pointer to GeographicalLocalityDistribution in order to take useful info,
			// like number of classes with the respective request rates.
			active = true;
			cModule* pSubModule = getParentModule()->getSubmodule("content_distribution");
			if (pSubModule)
			{
				glPointer = dynamic_cast<GeographicalLocalityContentDistribution*>(pSubModule);
				if (glPointer)
				{

					region_id = glPointer->get_region(getNodeIndex());
					cout << "Assigned Region to Client "<< getNodeIndex()<<":\t" << region_id << "\n";

					// The identifier of each msg will correspond to the region_id .
					arrival = new cMessage("arrival",ARRIVAL);
					scheduleAt( simTime() + exponential(1./glPointer->regionInfo->operator [](region_id).lambda), arrival);


					//lambda = getAncestorPar("lambda");		// Unused (the lambda is calculated for each class according to the configuration file).
					
					lambda = glPointer->regionInfo->operator [](region_id).lambda;
					check_time	= getAncestorPar("check_time");
					timer = new cMessage("timer", TIMER);
					scheduleAt( simTime() + check_time, timer );
				}
				else
				{
					// ERROR MSG: client_GeographicalLocality is strictly associated to the GeographicalLocalityContentDistribution.
					std::stringstream ermsg;
					ermsg<<"A 'Geographical Locality' client type requires to instantiate a 'Geographical Locality' content distribution"<<
							" type. Please check.";
					severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
				}
			}
			client::initialize();
		}
    }
}

int client_GeographicalLocality::numInitStages() const
{
	return 2;
}


void client_GeographicalLocality::finish()
{
	client::finish();
}


void client_GeographicalLocality::handleMessage(cMessage *in)
{
    if (in->isSelfMessage())	// A self-generated message can be either an 'ARRIVAL' or a 'TIMER'.
    {
    	short msgID = in->getKind();

    	if(msgID == TIMER)
    	{
			handle_timers(in);
			scheduleAt( simTime() + check_time, timer );
			return;
    	}
    	else       // It can be a request for content in the region assigend to this client.
    	{
			request_file();  	// Request contents 

			// Re-schedule a request according to the lambda of the 'msgID' class.
			scheduleAt( simTime() + exponential(1./glPointer->regionInfo->operator [](region_id).lambda), arrival);
    			
    		

    		return;
    	}
    }

    switch (in->getKind())	// In case of an external message, it can only be a DATA packet.
  	{
  		case CCN_D:
  		{
  			#ifdef SEVERE_DEBUG
  			if (!active){
  				std::stringstream ermsg;
  				ermsg<<"Client attached to node "<<getNodeIndex()<<
  					"  received a DATA despite being NOT ACTIVE";
  				severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
  			}
  			#endif

  			ccn_data *data_message = (ccn_data *) in;
  			handle_incoming_chunk (data_message);
  			delete  data_message;
  			break;
  		}

  		#ifdef SEVERE_DEBUG
  		default:
  			std::stringstream ermsg;
  			ermsg<<"Clients can only receive DATA, while this is a message"<<
  				" of TYPE "<<in->getKind();
  			severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
  		#endif
     }
}


void client_GeographicalLocality::request_file()
{
	// Extract a 'local' content ID from the range associated to assgined region.
	name_t nameGlobal = glPointer->regionInfo->operator[](region_id).contents_order.operator[](glPointer->zipfRegions.operator [](region_id)->sample()-1);

	struct download new_download = download (0,simTime() );

	#ifdef SEVERE_DEBUG
	new_download.serial_number = interests_sent;
	if (!active){
		std::stringstream ermsg;
		ermsg<<"Client attached to node "<< getNodeIndex() <<" is requesting file but it "
				<<"shoud not as it is not active";
		severe_error(__FILE__,__LINE__,ermsg.str().c_str() );
	}
	#endif

	current_downloads.insert(pair<name_t, download >(nameGlobal, new_download ) );
	send_interest(nameGlobal, 0 ,-1);

}

