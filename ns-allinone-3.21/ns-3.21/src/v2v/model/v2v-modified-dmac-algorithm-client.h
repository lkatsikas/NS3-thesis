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

#ifndef V2V_MODIFIED_DMAC_ALGORITHM_CLIENT_H_
#define V2V_MODIFIED_DMAC_ALGORITHM_CLIENT_H_

#include <map>
#include "ns3/ptr.h"
#include "ns3/double.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include "ns3/application.h"
#include "ns3/traced-callback.h"
#include "ns3/mobility-module.h"
#include "ns3/v2v-cluster-sap.h"
#include "ns3/v2v-cluster-header.h"

namespace ns3 {

class Socket;
class Address;

class V2vModifiedDMACAlgorithmClient: public Application {
public:

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);

    V2vModifiedDMACAlgorithmClient();
    virtual ~V2vModifiedDMACAlgorithmClient ();

    /**
     * \return pointer to listening socket
     */
    Ptr<Socket> GetListeningSocket (void) const;

    /**
     * \brief Return a pointer to associated socket.
     * \return pointer to associated socket
     */
    Ptr<Socket> GetSocket (void) const;

    /**
     * @brief GetClusterMembers
     * @return
     */
    double GetClusterMembers(void);

    /**
     * @brief GetFormationDelay
     * @return
     */
    double GetFormationDelay(void);

    /**
     * @brief GetClusterChanges
     * @return
     */
    uint64_t GetClusterChanges(void);

    /**
     * @brief GetNumberOfMessages
     * @return
     */
    uint64_t GetNumberOfMessages(void);

    /**
     * @brief GetRole
     * @return
     */
    V2vClusterSap::NovelNodeDegree GetRole(void);

    /**
     * @brief GetNumberOfMessagesPerSecond
     * @return
     */
    uint64_t GetNumberOfMessagesPerSecond(void);

    /**
     * @brief GetClusterChangesPerSecond
     * @return
     */
    uint64_t GetClusterChangesPerSecond(void);

    /**
     * @brief GetChDuration
     * @return
     */
    double GetChDuration(void);

    /**
     * @brief GetChDuration
     * @return
     */
    double GetCmDuration(void);

    /**
     * @brief GetMinChDuration
     * @return
     */
    double GetMinChDuration(void);

    /**
     * @brief GetMaxChDuration
     * @return
     */
    double GetMaxChDuration(void);

    /**
     * @brief GetMinCmDuration
     * @return
     */
    double GetMinCmDuration(void);

    /**
     * @brief GetMaxCmDuration
     * @return
     */
    double GetMaxCmDuration(void);


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
     * @brief ScheduleTransmitHello
     * @param dt
     */
    void ScheduleTransmitHello (Time dt);

    /**
     * @brief SendHello
     */
    void SendHello (void);

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
     * @brief ScheduleMaintenance
     * @param dt
     */
    void ScheduleMaintenance (Time dt);

    /**
     * @brief ApplyMaintenance
     */
    void ApplyMaintenance(void);

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
     * @brief Acquire current mobility info
     */
    void AcquireMobilityInfo (void);

    /**
     * @brief CreateNeighbor
     * @param ts
     * @param helloMessage
     * @return
     */
    V2vClusterSap::DMACNeighbours CreateNeighbor(Time ts, V2vClusterSap::DMACHello helloMessage);

    /**
     * @brief CreateHelloPacket
     * @return a DMACHello structure with content
     */
    V2vClusterSap::DMACHello CreateHelloPacket(void);

    /**
     * @brief CreateCHPacket
     * @return
     */
    V2vClusterSap::DMACCH CreateCHPacket(void);

    /**
     * @brief CreateJoinPacket
     * @return
     */
    V2vClusterSap::DMACJoin CreateJoinPacket(void);

    /**
     * @brief CreateForwardedHelloPacket
     * @param oldMessage
     */
    void CreateForwardedHelloPacket(V2vClusterSap::DMACHello oldMessage);

    /**
     * @brief CreateForwardedCHPacket
     * @param oldMessage
     */
    void CreateForwardedCHPacket(V2vClusterSap::DMACCH oldMessage);

    /**
     * @brief CreateForwardedJoinPacket
     * @param oldMessage
     */
    void CreateForwardedJoinPacket(V2vClusterSap::DMACJoin oldMessage);

    /**
     * @brief IsSameDirection
     * @param otherVelocity
     * @return
     */
    bool IsSameDirection(Vector otherVelocity);

    template <typename T> int sgn(T val);

    /**
     * @brief CalculateBeats
     * @return
     */
    double CalculateFreshness (Vector otherPosition, Vector otherVelocity);

    /**
     * @brief SolveQuadratic
     * @param a
     * @param b
     * @param c
     * @return
     */
    double SolveQuadratic(double a, double b, double c);

    /**
     * @brief TestClusterHeadChange
     * @return
     */
    bool TestClusterHeadChange(uint64_t id, V2vClusterSap::DMACNeighbours neighbor);

    /**
     * @brief CalculateWeight
     * @return
     */
    double CalculateWeight(void);

    /**
     * @brief CreateClusterNode
     * @param neighbor
     * @return
     */
    V2vClusterSap::DMACNeighbours CreateClusterNode(Time ts, V2vClusterSap::DMACNeighbours neighbor);

    /**
     * @brief ChooseClusterHead
     * @return
     */
    uint64_t ChooseClusterHead(void);

    /**
     * \brief Report the status of the node
     */
    void StatusReport (void);

    /**
     * @brief ChangeState
     * @param currentState
     */
    void ChangeState (V2vClusterSap::DMACNodeState currentState);


    /* Receive Socket */
    TypeId m_tidListening;          		//!< Protocol TypeId
    Address m_peerListening;       	 		//!< Local address to bind to
    uint32_t m_receivedCounter;             //!< Counter for sent packets
    Ptr<Socket> m_socketListening;      	//!< Listening socket

    /* Send Socket */
    TypeId m_tid;          					//!< Type of the socket used
    Address m_peer;        	 				//!< Peer address
    EventId m_sendEvent;    				//!< Event id of pending "send packet" event
    EventId m_sendHelloEvent;               //!< Event id of pending "send hello packet" event
    EventId m_maintenanceEvent;               //!< Event id of pending "maintenance tasks" event
    Ptr<Socket> m_socket;       			//!< Associated socket
    uint32_t m_sentCounter; 				//!< Counter for sent packets

    /* Channels Access Params */
    Time m_interval; 						//!< Packet inter-send time
    uint32_t m_maxUes;                      //!< maximun number of ues
    double m_minimumTdmaSlot;               //!< the minimum tdma slot
    double m_vehicleTdmaSlot;               //!< the timeslot for the node to schedule transmission
    double m_trainingPeriod;

    /* Clustering Params */
    uint16_t m_ttl;
    double m_beats;
    double m_range;
    double m_freshness;
    double m_freshnessThreshold;
    Ptr<MobilityModel> m_mobilityModel;
    V2vClusterSap::DMACNodeState m_nodeState;
    V2vClusterSap::DMACCurrentInfo m_currentInfo;
    std::map<uint64_t, V2vClusterSap::DMACNeighbours> m_clusterMap;
    std::map<uint64_t, V2vClusterSap::DMACNeighbours> m_neighborMap;

    V2vClusterSap::DMACHello m_forwardHello;
    V2vClusterSap::DMACCH m_forwardCH;
    V2vClusterSap::DMACJoin m_forwardJoin;

    /* Traces */
    TracedCallback<Ptr<const Packet> > m_txTrace;
    TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;


    /* Formation Delay metrics */
    uint64_t m_startFormationDelay;
    uint64_t m_stopFormationDelay;
    std::vector<uint64_t> m_formationDelayVector;

    /* Average Number of members per cluster */
    std::vector<uint64_t> m_clusterMembers;

    /* Number of Cluster Changes */
    uint64_t m_clusterChanges;

    /* Number of messages */
    uint64_t m_numberOfMessages;

    /* Number of messages per second */
    uint64_t m_numberOfMessagesPerSec;

    /* Cluster changes per second */
    uint64_t m_clusterChangesPerSec;

    /* Cluster Head duration metrics */
    bool m_chDurationBoolean;
    uint64_t m_startChDuration;
    uint64_t m_stopChDuration;
    std::vector<uint64_t> m_chDurationVector;

    /* Cluster Member duration metrics */
    bool m_cmDurationBoolean;
    uint64_t m_startCmDuration;
    uint64_t m_stopCmDuration;
    std::vector<uint64_t> m_cmDurationVector;

};

} // namespace ns3

#endif /* V2V_MODIFIED_DMAC_ALGORITHM_CLIENT_H_ */
