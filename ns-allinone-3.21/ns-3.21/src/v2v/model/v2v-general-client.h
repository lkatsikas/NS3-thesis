/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of Athens (UOA)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  - Lampros Katsikas <lkatsikas@di.uoa.gr>
 */

#ifndef V2V_GENERAL_CLIENT_H_
#define V2V_GENERAL_CLIENT_H_

#include <map>
#include "ns3/ptr.h"
#include "ns3/double.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include "ns3/application.h"
#include "ns3/traced-callback.h"
#include "ns3/v2v-mobility-model.h"
#include "ns3/v2v-cluster-sap.h"
#include "ns3/v2v-cluster-header.h"

namespace ns3 {

class Socket;
class Address;

class V2vGeneralClient: public Application {
public:

	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
    static TypeId GetTypeId (void);

    V2vGeneralClient();
    virtual ~V2vGeneralClient ();

	/**
	 * \return pointer to listening socket
	 */
    Ptr<Socket> GetListeningSocket (void) const;

	/**
	 * \brief Return a pointer to associated socket.
	 * \return pointer to associated socket
	 */
    Ptr<Socket> GetSocket (void) const;

protected:
    virtual void DoDispose (void);

private:
	/// inherited from Application base class.
    virtual void StartApplication (void);    // Called at time specified by Start
    virtual void StopApplication (void);     // Called at time specified by Stop

    void StartListeningLocal (void);	// Called from StartApplication()
    void StopListeningLocal (void);	// Called from StopApplication()

    /**
     * \brief Print sent/received packets statistics.
     */
    void PrintStatistics (std::ostream &os);


    //!< Receive locally
	/**
	 * \brief Handle a packet received by the application
	 * \param socket the receiving socket
	 */
    void HandleRead (Ptr<Socket> socket);
	/**
	 * \brief Handle an incoming connection
	 * \param socket the incoming connection socket
	 * \param from the address the connection is from
	 */
    void HandleAccept (Ptr<Socket> socket, const Address& from);
	/**
	 * \brief Handle an connection close
	 * \param socket the connected socket
	 */
    void HandlePeerClose (Ptr<Socket> socket);
	/**
	 * \brief Handle an connection error
	 * \param socket the connected socket
	 */
    void HandlePeerError (Ptr<Socket> socket);


    //!< Send locally
	/**
	 * \brief Schedule the next packet transmission
	 * \param dt time interval between packets.
	 */
    void ScheduleTransmit (Time dt);

	/**
	 * \brief Send a packet
	 */
    void Send (void);

	/**
	 * \brief Handle a Connection Succeed event
	 * \param socket the connected socket
	 */
    void ConnectionSucceeded (Ptr<Socket> socket);

	/**
	 * \brief Handle a Connection Failed event
	 * \param socket the not connected socket
	 */
    void ConnectionFailed (Ptr<Socket> socket);


    //!< Clustering Functions
	/**
	 * \brief Report the status of the node
	 */
    void StatusReport (void);

    /**
     * @brief Acquire current mobility info
     */
    void AcquireMobilityInfo (void);



	/* Receive Socket */
	TypeId m_tidListening;          		//!< Protocol TypeId
	Address m_peerListening;       	 		//!< Local address to bind to
    uint32_t m_receivedCounter;             //!< Counter for sent packets
    Ptr<Socket> m_socketListening;      	//!< Listening socket

	/* Send Socket */
	TypeId m_tid;          					//!< Type of the socket used
	Address m_peer;        	 				//!< Peer address
	EventId m_sendEvent;    				//!< Event id of pending "send packet" event
	Ptr<Socket> m_socket;       			//!< Associated socket
    uint32_t m_sentCounter; 				//!< Counter for sent packets

    /* Channels Access Params */
    Time m_interval; 						//!< Packet inter-send time
    uint32_t m_maxUes;                      //!< maximun number of ues
    double m_minimumTdmaSlot;               //!< the minimum tdma slot
    double m_vehicleTdmaSlot;               //!< the timeslot for the node to schedule transmission

    /* Clustering Params */
    Ptr<V2vMobilityModel> m_mobilityModel;
    V2vClusterSap::NovelNeighborInfo m_currentMobility;

    /* Traces */
    TracedCallback<Ptr<const Packet> > m_txTrace;
    TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

};

} // namespace ns3

#endif /* V2V_GENERAL_CLIENT_H_ */
