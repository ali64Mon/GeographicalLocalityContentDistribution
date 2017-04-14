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

#ifndef CLIENT_GEOGRAPHICALLOCALITY_H_
#define CLIENT_GEOGRAPHICALLOCALITY_H_

#include <omnetpp.h>
#include <random>
#include "ccnsim.h"
#include "client.h"
#include "GeographicalLocalityContentDistribution.h"


using namespace std;


/*
 * this client generates requests according to the inofrmation of a distribution of a region to which it is assigned. 
*/

class client_GeographicalLocality : public virtual client {
	friend class client_ls_GeographicalLocality;
	public:
		virtual void initialize(int);				// Multi-stage initialization.
		int numInitStages() const;
		virtual void handleMessage(cMessage *);
		virtual void finish();

		virtual void request_file();
		bool validateRequest(int, name_t);

    private:
		static GeographicalLocalityContentDistribution* glPointer;		// Pointer to the ShotNoiseContentDistribution class.

		cMessage * arrival;		// There is a message for each popularity class in order to trigger the respective
											// request process.
		cMessage *timer;

		vector<bool> onOffClass;			// Vector indicating which class will be modeled as ON-OFF
											// (e.g., if false, requests for that class will be always generated).

		int region_id;
};
#endif