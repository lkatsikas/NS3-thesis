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

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/string.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket.h"
#include "ns3/address-utils.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "v2v-novel-algorithm-client.h"

#include "ns3/random-variable.h"


namespace ns3 {

static const std::string
StateName[V2vClusterSap::NOVEL_STATES] =
{
    "NONE",
    "INITIAL",
    "COV",
    "FORMATION",
    "UPDATE"
};

static const std::string & ToString (V2vClusterSap::NovelNodeState state){
    return StateName[state];
}

static const std::string
DegreeName[V2vClusterSap::DEGREE_STATES] =
{
    "STANDALONE",
    "CH",
    "CM"
};

static const std::string & ToString (V2vClusterSap::NovelNodeDegree NovelNodeDegree){
    return DegreeName[NovelNodeDegree];
}

NS_LOG_COMPONENT_DEFINE ("V2vNovelAlgorithmClient");
NS_OBJECT_ENSURE_REGISTERED (V2vNovelAlgorithmClient);

TypeId V2vNovelAlgorithmClient::GetTypeId(void) {
	static TypeId tid =
            TypeId("ns3::V2vNovelAlgorithmClient").SetParent<Application>()
            .AddConstructor<V2vNovelAlgorithmClient>()
            .AddAttribute("ListeningLocal",
					"The Address on which to Bind the rx socket.",
                    AddressValue(), MakeAddressAccessor(&V2vNovelAlgorithmClient::m_peerListening),
					MakeAddressChecker())
            .AddAttribute("ProtocolListeningLocal",
					"The type id of the protocol to use for the rx socket.",
					TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vNovelAlgorithmClient::m_tidListening),
					MakeTypeIdChecker())
            .AddTraceSource("RxLocal", "A packet has been received",
                    MakeTraceSourceAccessor(&V2vNovelAlgorithmClient::m_rxTrace))

            .AddAttribute("TrainingPeriod",
                    "Training period of time", DoubleValue(80.0),
                    MakeDoubleAccessor(&V2vNovelAlgorithmClient::m_trainingPeriod),
                    MakeDoubleChecker<double>())
            .AddAttribute("ClusterTimeMetric",
                    "The maximun size of the TDMA window", DoubleValue(0.1),
                    MakeDoubleAccessor(&V2vNovelAlgorithmClient::m_clusterTimeMetric),
                    MakeDoubleChecker<double>())
            .AddAttribute("VehicleTdmaSlot",
                    "The vehicle's' TDMA window", DoubleValue(0.1),
                    MakeDoubleAccessor(&V2vNovelAlgorithmClient::m_vehicleTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("MinimumTdmaSlot",
                    "The maximun size of the TDMA window", DoubleValue(0.001),
                    MakeDoubleAccessor(&V2vNovelAlgorithmClient::m_minimumTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("MaxUes",
                    "The maximun size of ues permitted", UintegerValue(100),
                    MakeUintegerAccessor(&V2vNovelAlgorithmClient::m_maxUes),
                    MakeUintegerChecker<uint32_t>(1))
            .AddAttribute ("Interval",
                    "The time to wait between packets", TimeValue (Seconds (1.0)),
                    MakeTimeAccessor (&V2vNovelAlgorithmClient::m_interval),
                    MakeTimeChecker ())
            .AddAttribute("SendingLocal",
					"The address of the destination", AddressValue(),
                    MakeAddressAccessor(&V2vNovelAlgorithmClient::m_peer),
					MakeAddressChecker())
            .AddAttribute("ProtocolSendingLocal",
					"The type of protocol for the tx socket.",
					TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vNovelAlgorithmClient::m_tid),
					MakeTypeIdChecker())
			.AddAttribute ("MobilityModel",
				    "The mobility model of the node.",
				    PointerValue (),
                    MakePointerAccessor (&V2vNovelAlgorithmClient::m_mobilityModel),
                    MakePointerChecker<MobilityModel> ())
            .AddTraceSource("TxLocal","A new packet is created and is sent",
                    MakeTraceSourceAccessor(&V2vNovelAlgorithmClient::m_txTrace));
	return tid;
}


// Public Members
V2vNovelAlgorithmClient::V2vNovelAlgorithmClient () {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    m_clusterChanges = 0;
    m_numberOfMessages = 0;
    m_clusterChangesPerSec = 0;

    m_sentCounter = 0;
    m_receivedCounter = 0;
    m_sendEvent = EventId ();
}

V2vNovelAlgorithmClient::~V2vNovelAlgorithmClient () {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    m_sentCounter = 0;
    m_receivedCounter = 0;
}

double
V2vNovelAlgorithmClient::GetClusterMembers(void){

    if(m_clusterMembers.size () > 0){

        double s = 0.0;
        for(std::vector<uint64_t>::iterator it = m_clusterMembers.begin (); it != m_clusterMembers.end (); ++it){
            s += (*it);
        }
        return s/m_clusterMembers.size ();
    }

    return 0;
}

double
V2vNovelAlgorithmClient::GetFormationDelay (void){

    if(m_formationDelayVector.size () > 0){

        uint64_t sumDelay = 0;
        for(std::vector<uint64_t>::iterator it = m_formationDelayVector.begin (); it != m_formationDelayVector.end (); ++it){
            sumDelay += *it;
        }
        return TimeStep (sumDelay/m_formationDelayVector.size ()).GetSeconds ();
    }

    return 0;
}

uint64_t
V2vNovelAlgorithmClient::GetClusterChanges (void){
    return m_clusterChanges;
}

uint64_t
V2vNovelAlgorithmClient::GetNumberOfMessages (void){
    return m_numberOfMessages;
}

V2vClusterSap::NovelNodeDegree
V2vNovelAlgorithmClient::GetRole (void){
    return m_currentInfo.degree;
}

uint64_t
V2vNovelAlgorithmClient::GetNumberOfMessagesPerSecond (void){
    uint64_t tmp = m_numberOfMessagesPerSec;
    m_numberOfMessagesPerSec = 0;

    return tmp;
}

uint64_t V2vNovelAlgorithmClient::GetClusterChangesPerSecond (void){
    uint64_t tmp = m_clusterChangesPerSec;
    m_clusterChangesPerSec = 0;

    return tmp;
}

double
V2vNovelAlgorithmClient::GetChDuration (void){

    if(m_chDurationBoolean){
        m_chDurationBoolean = false;
        m_stopChDuration = Simulator::Now ().GetTimeStep ();
        m_chDurationVector.push_back (m_stopChDuration - m_startChDuration);
    }

    if(m_chDurationVector.size () > 0){

        uint64_t sumDuration = 0;
        for(std::vector<uint64_t>::iterator it = m_chDurationVector.begin (); it != m_chDurationVector.end (); ++it){
            sumDuration += *it;
        }
        return TimeStep (sumDuration/m_chDurationVector.size ()).GetSeconds ();
    }

    return 0;
}

double
V2vNovelAlgorithmClient::GetCmDuration (void){

    if(m_cmDurationBoolean){
        m_cmDurationBoolean = false;
        m_stopCmDuration = Simulator::Now ().GetTimeStep ();
        m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
    }

    if(m_cmDurationVector.size () > 0){

        uint64_t sumDuration = 0;
        for(std::vector<uint64_t>::iterator it = m_cmDurationVector.begin (); it != m_cmDurationVector.end (); ++it){
            sumDuration += *it;
        }
        return TimeStep (sumDuration/m_cmDurationVector.size ()).GetSeconds ();
    }

    return 0;
}

double
V2vNovelAlgorithmClient::GetMinChDuration (void){

    uint64_t min = Simulator::Now ().GetTimeStep ();
    for(std::vector<uint64_t>::iterator it = m_chDurationVector.begin (); it != m_chDurationVector.end (); ++it){
        if((*it) < min){
            min = *it;
        }
    }
    return TimeStep (min).GetSeconds ();
}

double
V2vNovelAlgorithmClient::GetMaxChDuration (void){

    uint64_t max = 0;
    for(std::vector<uint64_t>::iterator it = m_chDurationVector.begin (); it != m_chDurationVector.end (); ++it){
        if((*it) > max){
            max = *it;
        }
    }
    return TimeStep (max).GetSeconds ();
}

double
V2vNovelAlgorithmClient::GetMinCmDuration (void){

    uint64_t min = Simulator::Now ().GetTimeStep ();
    for(std::vector<uint64_t>::iterator it = m_cmDurationVector.begin (); it != m_cmDurationVector.end (); ++it){
        if((*it) < min){
            min = *it;
        }
    }
    return TimeStep (min).GetSeconds ();
}

double
V2vNovelAlgorithmClient::GetMaxCmDuration (void){

    uint64_t max = 0;
    for(std::vector<uint64_t>::iterator it = m_cmDurationVector.begin (); it != m_cmDurationVector.end (); ++it){
        if((*it) > max){
            max = *it;
        }
    }
    return TimeStep (max).GetSeconds ();
}

void
V2vNovelAlgorithmClient::PrintStatistics (std::ostream &os){

    os << "********************************************************" << std::endl
       << "  - Cluster Metrics - for"  << " Node:" << m_currentInfo.id  << std::endl
       << " Overall Sent messages: " << m_sentCounter << std::endl
       << " Overall Received messages: " << m_receivedCounter << std::endl
       << "********************************************************" << std::endl;


    os << "\n********************************************************" << std::endl
       << "  - Average Cluster Formation Delay:" << GetFormationDelay() <<  std::endl
       << " ------------------------------------------------------" << std::endl;

     os << "  - Average Number of members in cluster:" << GetClusterMembers () <<  std::endl
        << " ------------------------------------------------------" << std::endl;

     os << "  - Number of Cluster Changes:" << m_clusterChanges <<  std::endl
        << " ------------------------------------------------------" << std::endl;

     os << "  - Number of Messages:" << m_numberOfMessages <<  std::endl
        << " ------------------------------------------------------" << std::endl;
}

// Protected Members
void
V2vNovelAlgorithmClient::DoDispose (void) {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

	// chain up
	Application::DoDispose();
}

void
V2vNovelAlgorithmClient::StartApplication (void)
{
    NS_LOG_FUNCTION (this);

	// Create the socket if not already
	if (!m_socket) {
		m_socket = Socket::CreateSocket(GetNode(), m_tid);
		if (Inet6SocketAddress::IsMatchingType(m_peer)) {
			m_socket->Bind6();
		} else if (InetSocketAddress::IsMatchingType(m_peer)
				|| PacketSocketAddress::IsMatchingType(m_peer)) {
			m_socket->Bind();
		}
		m_socket->Connect(m_peer);
		m_socket->SetAllowBroadcast(true);
		m_socket->ShutdownRecv();

		m_socket->SetConnectCallback(
                MakeCallback(&V2vNovelAlgorithmClient::ConnectionSucceeded, this),
                MakeCallback(&V2vNovelAlgorithmClient::ConnectionFailed, this));
    }

    if(m_maxUes > 100){
        NS_FATAL_ERROR("Error: Maximum number of ues is 100.");
    }

    StartListeningLocal();

    ScheduleUpdate (Seconds(m_trainingPeriod - m_vehicleTdmaSlot));
    ScheduleTransmit (Seconds (m_trainingPeriod + m_vehicleTdmaSlot));
}

void
V2vNovelAlgorithmClient::StartListeningLocal (void)    // Called at time specified by Start
{
	NS_LOG_FUNCTION (this);

	// Create the socket if not already
	if (!m_socketListening) {
		m_socketListening = Socket::CreateSocket(GetNode(), m_tidListening);
		m_socketListening->Bind(m_peerListening);
		m_socketListening->Listen();
		m_socketListening->ShutdownSend();
		if (addressUtils::IsMulticast(m_peerListening)) {
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socketListening);
			if (udpSocket) {
				// equivalent to setsockopt (MCAST_JOIN_GROUP)
				udpSocket->MulticastJoinGroup(0, m_peerListening);
			} else {
				NS_FATAL_ERROR("Error: joining multicast on a non-UDP socket");
			}
		}
	}

    m_socketListening->SetRecvCallback(MakeCallback(&V2vNovelAlgorithmClient::HandleRead, this));
	m_socketListening->SetAcceptCallback(
			MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback(&V2vNovelAlgorithmClient::HandleAccept, this));
	m_socketListening->SetCloseCallbacks(
            MakeCallback(&V2vNovelAlgorithmClient::HandlePeerClose, this),
            MakeCallback(&V2vNovelAlgorithmClient::HandlePeerError, this));
}

void
V2vNovelAlgorithmClient::StopApplication (void) // Called at time specified by Stop
{
	NS_LOG_FUNCTION (this);

	if (m_socket != 0) {
		m_socket->Close();
		m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
		m_socket = 0;
	} else {
        NS_LOG_WARN ("V2vNovelAlgorithmClient found null socket to close in StopApplication");
	}
    Simulator::Cancel (m_sendEvent);
    Simulator::Cancel (m_updateEvent);
    StopListeningLocal();

    StatusReport ();
    PrintStatistics(std::cout);
}

void
V2vNovelAlgorithmClient::StopListeningLocal (void)     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  if (m_socketListening)
    {
	  m_socketListening->Close ();
	  m_socketListening->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	  m_socketListening = 0;
    }
}

Ptr<Socket>
V2vNovelAlgorithmClient::GetListeningSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socketListening;
}

Ptr<Socket>
V2vNovelAlgorithmClient::GetSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socket;
}


// Private Members
void
V2vNovelAlgorithmClient::HandleRead (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from))) {
        if (packet->GetSize() == 0) { //EOF
            break;
        }

        Ptr<Packet> p = packet->Copy ();
        PacketMetadata::ItemIterator metadataIterator = p->BeginItem();
        PacketMetadata::Item item;
        while (metadataIterator.HasNext()){
            item = metadataIterator.Next();

            if(item.tid.GetName() == "ns3::V2vNovelCOVHeader"){
                V2vNovelCOVHeader covHeader;
                p->RemoveHeader (covHeader);

                std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator cov = m_stableNeighborMap.find(covHeader.GetTempClusterId ());
                if(cov != m_stableNeighborMap.end ()){

                    if(m_currentInfo.tempClusterId == 0){

                        m_covVelocity = cov->second.velocity;
                        if(IsLowerThan(m_covVelocity, m_currentInfo.velocity)){

                            ChangeState (m_nodeState);
                            ChangeState (m_nodeState);
                            m_currentInfo.tempClusterId = covHeader.GetTempClusterId ();

                            double waitingTime = SuitabilityCheck();
                            NS_LOG_UNCOND ("NodeId: " << m_currentInfo.id << " WaitingTime is: " << waitingTime);
                            ScheduleTransmit (Seconds(waitingTime));
                        }
                    }
                }
            }
            else if(item.tid.GetName() == "ns3::V2vNovelFormationHeader"){
                V2vNovelFormationHeader formationHeader;
                p->RemoveHeader (formationHeader);

                std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator ch = m_stableNeighborMap.find(formationHeader.GetClusterId());
                if(m_currentInfo.tempClusterId == formationHeader.GetTempClusterId()){
                    if((ch != m_stableNeighborMap.end ()) && (m_currentInfo.clusterId == 0)){

                        if(IsLowerThan(m_covVelocity, m_currentInfo.velocity)){
                            m_currentInfo.degree = V2vClusterSap::CM;
                            m_currentInfo.clusterId = formationHeader.GetClusterId();

                            m_sendEvent.Cancel ();
                            NS_LOG_DEBUG ("Cluster Head election event cancelled...Turning to CM");

                            ChangeState (m_nodeState);

                            m_clusterChanges ++;
                            m_clusterChangesPerSec ++;

                            if(m_chDurationBoolean){
                                m_chDurationBoolean = false;
                                m_stopChDuration = Simulator::Now ().GetTimeStep ();
                                m_chDurationVector.push_back (m_stopChDuration - m_startChDuration);
                            }

                            if(m_cmDurationBoolean){
                                m_cmDurationBoolean = false;
                                m_stopCmDuration = Simulator::Now ().GetTimeStep ();
                                m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
                            }

                            if(m_cmDurationBoolean == false){
                                m_startCmDuration = Simulator::Now ().GetTimeStep ();
                                m_cmDurationBoolean = true;
                            }
                        }
                    }
                }
            }
            else if(item.tid.GetName() == "ns3::V2vNovelUpdateHeader"){
                V2vNovelUpdateHeader updateHeader;
                p->RemoveHeader (updateHeader);

                if(IsSameDirection (updateHeader.GetUpdateInfo ().velocity)){

                    m_neighborMap[updateHeader.GetUpdateInfo().id] = updateHeader.GetUpdateInfo ();
                    if(IsStable (updateHeader.GetUpdateInfo ().velocity)){
                        m_stableNeighborMap[updateHeader.GetUpdateInfo().id] = updateHeader.GetUpdateInfo ();
                    }
                    if(m_currentInfo.id == updateHeader.GetUpdateInfo ().clusterId){
                        m_clusterMap[updateHeader.GetUpdateInfo().id] = updateHeader.GetUpdateInfo ();

                        //!< Store Address of cluster members
                        m_membersAddress[updateHeader.GetUpdateInfo().id] = from;
                    }
                }
            }
            else if(item.tid.GetName() == "ns3::V2vNovelMergeHeader"){
                V2vNovelMergeHeader mergeHeader;
                p->RemoveHeader (mergeHeader);

                // Invalidate old CH
                std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator oldCH = m_stableNeighborMap.find(mergeHeader.GetOldClusterId ());
                if(oldCH != m_stableNeighborMap.end ()){
                    oldCH->second.degree = V2vClusterSap::CM;
                    oldCH->second.clusterId = mergeHeader.GetNewClusterId ();
                }

                if(m_currentInfo.clusterId == mergeHeader.GetOldClusterId ()){

                    std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator newCH = m_stableNeighborMap.find(mergeHeader.GetNewClusterId ());
                    if(newCH != m_stableNeighborMap.end ()){
                        NS_LOG_UNCOND("New clusterhead is: " << mergeHeader.GetNewClusterId ());
                        m_currentInfo.degree = V2vClusterSap::CM;
                        m_currentInfo.clusterId = mergeHeader.GetNewClusterId ();

                        m_clusterChanges ++;
                        m_clusterChangesPerSec ++;

                        if(m_cmDurationBoolean){
                            m_cmDurationBoolean = false;
                            m_stopCmDuration = Simulator::Now ().GetTimeStep ();
                            m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
                        }

                        if(m_cmDurationBoolean == false){
                            m_startCmDuration = Simulator::Now ().GetTimeStep ();
                            m_cmDurationBoolean = true;
                        }
                    }
                    else{
                        uint64_t suitableNeighbour = FindStableClusterHead();
                        std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator nextCH = m_stableNeighborMap.find(suitableNeighbour);
                        if(nextCH != m_stableNeighborMap.end ()){

                            //!< Join to another suitable neighbour
                            m_maintenanceCounter ++;
                            m_currentInfo.degree = V2vClusterSap::CM;
                            m_currentInfo.clusterId = nextCH->second.clusterId;

                            m_clusterChanges ++;
                            m_clusterChangesPerSec ++;


                            if(m_cmDurationBoolean){
                                m_cmDurationBoolean = false;
                                m_stopCmDuration = Simulator::Now ().GetTimeStep ();
                                m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
                            }

                            if(m_cmDurationBoolean == false){
                                m_startCmDuration = Simulator::Now ().GetTimeStep ();
                                m_cmDurationBoolean = true;
                            }
                        }
                        else{

                            //!< Leave the cluster
                            m_maintenanceCounter ++;
                            m_currentInfo.clusterId = 0;
                            m_currentInfo.degree = V2vClusterSap::STANDALONE;

                            if(m_cmDurationBoolean){
                                m_cmDurationBoolean = false;
                                m_stopCmDuration = Simulator::Now ().GetTimeStep ();
                                m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
                            }

                            NS_LOG_DEBUG ("Cannot follow merge...Go to STANDALONE state: " << m_currentInfo.id);
                        }
                    }
                }
            }
            m_rxTrace(p, from);
        }
        m_receivedCounter ++;
    }
}

void
V2vNovelAlgorithmClient::HandleAccept (Ptr<Socket> s, const Address& from) {
	NS_LOG_FUNCTION (this << s << from);
    s->SetRecvCallback(MakeCallback(&V2vNovelAlgorithmClient::HandleRead, this));
}

void
V2vNovelAlgorithmClient::HandlePeerClose (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}

void
V2vNovelAlgorithmClient::HandlePeerError (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}



void
V2vNovelAlgorithmClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &V2vNovelAlgorithmClient::Send, this);
}

void
V2vNovelAlgorithmClient::Send (void) {

    NS_LOG_FUNCTION (this);
    //NS_ASSERT(m_sendEvent.IsExpired());

    switch (m_nodeState) {
    case V2vClusterSap::UNDEFINED:{

        V2vNovelUpdateHeader updateHeader;
        updateHeader.SetUpdateInfo(m_currentInfo);

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader(updateHeader);
        m_txTrace(packet);
        m_socket->Send(packet);
        ++ m_sentCounter;
        ++ m_formationCounter;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends Update Message at : " << updateHeader.GetUpdateInfo ().ts);

        ChangeState (m_nodeState);
        ScheduleTransmit (Seconds(m_maxUes*m_minimumTdmaSlot));
        ScheduleMaintenance(Seconds(m_maxUes*m_minimumTdmaSlot*10));

        break;
    }
    case V2vClusterSap::INITIAL:{

        if(m_formationDelayBoolean == false){
            m_startFormationDelay = Simulator::Now ().GetTimeStep ();
        }

        if(IsSlowestVehicle()){
            ChangeState (m_nodeState);
            ScheduleTransmit (Seconds(m_maxUes*m_minimumTdmaSlot));
        }
        else{
            Simulator::Schedule (m_interval, &V2vNovelAlgorithmClient::Check, this);
            m_formationDelayBoolean = true;
        }

        break;
    }
    case V2vClusterSap::COV:{

        m_currentInfo.tempClusterId = m_currentInfo.id;

        V2vNovelCOVHeader covHeader;
        covHeader.SetTempClusterId(m_currentInfo.tempClusterId);

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader(covHeader);
        m_txTrace(packet);
        m_socket->Send(packet);
        ++ m_sentCounter;
        ++ m_formationCounter;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends COV Message at : " << covHeader.GetTs());
        ChangeState (m_nodeState);

        double waitingTime = SuitabilityCheck ();
        NS_LOG_UNCOND ("NodeId: " << m_currentInfo.id << " WaitingTime is: " << waitingTime);

        ScheduleTransmit (Seconds(waitingTime));

        break;
    }
    case V2vClusterSap::FORMATION:{

        m_currentInfo.degree = V2vClusterSap::CH;
        m_currentInfo.clusterId = m_currentInfo.id;

        V2vNovelFormationHeader formationHeader;
        formationHeader.SetClusterId(m_currentInfo.clusterId);
        formationHeader.SetTempClusterId(m_currentInfo.tempClusterId);

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader(formationHeader);
        m_txTrace(packet);
        m_socket->Send(packet);
        ++ m_sentCounter;
        ++ m_formationCounter;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends Cluster Formation Message at : " << formationHeader.GetTs());

        ChangeState (m_nodeState);


        m_stopFormationDelay = Simulator::Now ().GetTimeStep ();
        m_formationDelayBoolean = false;
        m_formationDelayVector.push_back (m_stopFormationDelay - m_startFormationDelay);


        if(m_chDurationBoolean){
            m_chDurationBoolean = false;
            m_stopChDuration = Simulator::Now ().GetTimeStep ();
            m_chDurationVector.push_back (m_stopChDuration - m_startChDuration);
        }

        if(m_cmDurationBoolean){
            m_cmDurationBoolean = false;
            m_stopCmDuration = Simulator::Now ().GetTimeStep ();
            m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
        }

        if(m_chDurationBoolean == false){
            m_startChDuration = Simulator::Now ().GetTimeStep ();
            m_chDurationBoolean = true;
        }

        m_clusterChanges ++;
        m_clusterChangesPerSec ++;

        break;
    }
    default:
        break;
    }
}

void
V2vNovelAlgorithmClient::ScheduleUpdate (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_updateEvent = Simulator::Schedule(dt, &V2vNovelAlgorithmClient::Update, this);
}

void
V2vNovelAlgorithmClient::Update(){

    CreateUpdateMessage ();
    V2vNovelUpdateHeader updateHeader;
    updateHeader.SetUpdateInfo(m_currentInfo);

    Ptr<Packet> packet = Create<Packet>(0);
    packet->AddHeader(updateHeader);
    m_txTrace(packet);
    m_socket->Send(packet);
    ++ m_sentCounter;

    m_numberOfMessages ++;
    m_numberOfMessagesPerSec ++;

    ScheduleUpdate (m_interval);
}

void
V2vNovelAlgorithmClient::ScheduleMaintenance(Time dt){
    NS_LOG_FUNCTION (this << dt);
    m_maintenanceEvent = Simulator::Schedule(dt, &V2vNovelAlgorithmClient::MaintenanceTasks, this);
}

void
V2vNovelAlgorithmClient::MaintenanceTasks (void){

    NS_LOG_DEBUG("Maintenance tasks");

    if(m_currentInfo.degree == V2vClusterSap::CH){
        m_clusterMembers.push_back (m_clusterMap.size ()+1);
    }

    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_stableNeighborMap.begin(); it != m_stableNeighborMap.end();){
        V2vClusterSap::NovelNeighborInfo value = it->second;
        if(TimeStep (m_currentInfo.ts).GetSeconds () - TimeStep (value.ts).GetSeconds () > 1.5){
            m_stableNeighborMap.erase (it++);
        }
        else{
            ++it;
        }
    }
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_stableNeighborMap.begin(); it != m_stableNeighborMap.end();){
        V2vClusterSap::NovelNeighborInfo value = it->second;
        if(TimeStep (m_currentInfo.ts).GetSeconds () - TimeStep (value.ts).GetSeconds () > 1.5){
            m_stableNeighborMap.erase (it++);
        }
        else{
            ++it;
        }
    }

    //!< Update Neighbor's List according to Timestamps
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end();){

        uint64_t key = it->first;
        V2vClusterSap::NovelNeighborInfo value = it->second;
        if(TimeStep (m_currentInfo.ts).GetSeconds () - TimeStep (value.ts).GetSeconds () > 1.5){
            NS_LOG_DEBUG ("At: " << Simulator::Now().GetSeconds() << " Node::" <<
                          m_currentInfo.id << " - Removing Node:" << value.id<< " from m_neighborMap with last sent time:"
                          << TimeStep (value.ts).GetSeconds ());

            if(m_stableNeighborMap.find(key) != m_stableNeighborMap.end()){
                NS_LOG_DEBUG ("At: " << Simulator::Now().GetSeconds() << " Node::" <<
                              m_currentInfo.id << " - Removing Node:" << value.id<< " from m_stableNeighborMap with last sent time:"
                              << TimeStep (value.ts).GetSeconds ());
                m_stableNeighborMap.erase(key);
            }

            if(m_clusterMap.find(key) != m_clusterMap.end()){
                NS_LOG_DEBUG ("At: " << Simulator::Now().GetSeconds() << " Node::" <<
                              m_currentInfo.id << " - Removing Node:" << value.id<< " from m_clusterMap with last sent time:"
                              << TimeStep (value.ts).GetSeconds ());
                m_clusterMap.erase(key);
            }
            m_neighborMap.erase(it++);

            //!< Leave Cluster Case
            if(m_currentInfo.clusterId == value.id){

                NS_LOG_DEBUG ("Node:" << m_currentInfo.id << " lost ClusterHead " << value.id);

                //!< Leave the cluster
                m_currentInfo.clusterId = 0;
                m_currentInfo.degree = V2vClusterSap::STANDALONE;

                NS_LOG_DEBUG ("Go to STANDALONE state: " << m_currentInfo.id);
            }
        }
        else{

            //!< Merge Clusters Case
            if ((m_currentInfo.degree == V2vClusterSap::CH) && (value.degree == V2vClusterSap::CH)){
                NS_LOG_UNCOND("Check possible merge between " << m_currentInfo.id << " and " << value.id);
                std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator chStable = m_stableNeighborMap.find(value.id);
                if(chStable != m_stableNeighborMap.end ()){

                    V2vClusterSap::NovelNeighborInfo neighbourCH = chStable->second;
                    if(m_currentInfo.chMembers < neighbourCH.chMembers){
                        NS_LOG_UNCOND("Merge Clusters => m_current.id:" << m_currentInfo.id << ", members:" << m_currentInfo.chMembers
                                      << " -- neighbourCH.id:" << neighbourCH.id << ", members:" << neighbourCH.chMembers);

                        RemoveMergeSocket ();
                        CreateMergeSocket();
                        MergeSend (neighbourCH.id);

                        m_currentInfo.degree = V2vClusterSap::CM;
                        m_currentInfo.clusterId = neighbourCH.clusterId;
                        m_clusterMap.clear ();
                        m_maintenanceCounter ++;

                        m_clusterChanges ++;
                        m_clusterChangesPerSec ++;

                        if(m_chDurationBoolean){
                            m_chDurationBoolean = false;
                            m_stopChDuration = Simulator::Now ().GetTimeStep ();
                            m_chDurationVector.push_back (m_stopChDuration - m_startChDuration);
                        }

                        if(m_cmDurationBoolean == false){
                            m_startCmDuration = Simulator::Now ().GetTimeStep ();
                            m_cmDurationBoolean = true;
                        }
                    }
                }
            }
            ++it;
        }
    }

    if((m_currentInfo.degree == V2vClusterSap::STANDALONE) && (m_nodeState == V2vClusterSap::UPDATE)){
        uint64_t suitableNeighbour = FindStableClusterHead();
        std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator nextCH = m_stableNeighborMap.find(suitableNeighbour);
        if(nextCH != m_stableNeighborMap.end ()){

            //!< Join to another suitable neighbour
            m_currentInfo.degree = V2vClusterSap::CM;
            m_currentInfo.clusterId = nextCH->second.clusterId;
            m_maintenanceCounter ++;

            if(m_cmDurationBoolean){
                m_cmDurationBoolean = false;
                m_stopCmDuration = Simulator::Now ().GetTimeStep ();
                m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
            }
        }
        else{

            //!< Leave the cluster
            m_maintenanceCounter ++;
            m_currentInfo.clusterId = 0;
            m_currentInfo.tempClusterId = 0;
            m_nodeState = V2vClusterSap::INITIAL;
            m_currentInfo.degree = V2vClusterSap::STANDALONE;

            if(m_cmDurationBoolean){
                m_cmDurationBoolean = false;
                m_stopCmDuration = Simulator::Now ().GetTimeStep ();
                m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
            }

            double timeSlot = 1.0 - (Simulator::Now ().GetSeconds ()- (int)Simulator::Now ().GetSeconds ()) + (m_maxUes*m_minimumTdmaSlot) + m_vehicleTdmaSlot;
            ScheduleTransmit (Seconds(timeSlot));

            NS_LOG_DEBUG ("Go to STANDALONE state: " << m_currentInfo.id);
        }
    }

    //StatusReport ();
    ScheduleMaintenance (m_interval);
}

void
V2vNovelAlgorithmClient::ConnectionSucceeded (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vNovelAlgorithmClient::ConnectionFailed (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}

void
V2vNovelAlgorithmClient::CreateMergeSocket (void){
    NS_LOG_FUNCTION (this);

    //!< Create merge socket with ClusterHead for merge event
    Ipv4Address mergeAddress = Ipv4Address::GetBroadcast ();
    uint16_t mergePort = InetSocketAddress::ConvertFrom (m_peer).GetPort ();
    Address memberAddress = Address(InetSocketAddress(mergeAddress, mergePort));

    // Create the socket if not already
    m_mergeSocket = Socket::CreateSocket(GetNode(), m_tid);
    if (Inet6SocketAddress::IsMatchingType(memberAddress)) {
        m_mergeSocket->Bind6();
    } else if (InetSocketAddress::IsMatchingType(memberAddress)
            || PacketSocketAddress::IsMatchingType(memberAddress)) {
        m_mergeSocket->Bind();
    }
    m_mergeSocket->Connect(memberAddress);
    m_mergeSocket->SetAllowBroadcast(true);
    m_mergeSocket->ShutdownRecv();
}

void
V2vNovelAlgorithmClient::RemoveMergeSocket (void){
    NS_LOG_FUNCTION (this);

    if (m_mergeSocket != 0) {
        m_mergeSocket->Close();
        m_mergeSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
        m_mergeSocket = 0;
    }
}

void
V2vNovelAlgorithmClient::MergeSend (uint64_t newCHId){

    V2vNovelMergeHeader mergeHeader;
    mergeHeader.SetNewClusterId(newCHId);
    mergeHeader.SetOldClusterId(m_currentInfo.id);

    Ptr<Packet> packet = Create<Packet>(0);
    packet->AddHeader(mergeHeader);
    m_mergeSocket->Send(packet);

    m_numberOfMessages ++;
    m_numberOfMessagesPerSec ++;

    NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Send Merge message to members");
}

void V2vNovelAlgorithmClient::CreateUpdateMessage (void){

    NS_LOG_DEBUG("Update mobility Info");

    //!< Acquire current mobility stats
    m_currentInfo.ts = Simulator::Now().GetTimeStep();
    m_currentInfo.id = this->GetNode ()->GetId ();
    m_currentInfo.chMembers = m_clusterMap.size ();
    m_currentInfo.position = m_mobilityModel->GetPosition();
    m_currentInfo.velocity = m_mobilityModel->GetVelocity();
    m_currentInfo.direction = Vector(0.0, 0.0, 0.0);//m_currentInfo.direction = m_mobilityModel->GetDirection();
}

void
V2vNovelAlgorithmClient::Check (void){
    if(m_currentInfo.tempClusterId == 0){
        ScheduleTransmit (Seconds(0.));
    }
}

bool
V2vNovelAlgorithmClient::IsSameDirection(Vector otherVelocity){

    if((m_currentInfo.velocity.x == 0.0) && (m_currentInfo.velocity.y == 0.0) && (otherVelocity.x == 0.0) && (otherVelocity.y == 0.0)) {
        NS_LOG_DEBUG("Same Direction");
        return true;
    }

    if((sgn(otherVelocity.x) == sgn(m_currentInfo.velocity.x)) && (sgn(otherVelocity.y) == sgn(m_currentInfo.velocity.y))){
        NS_LOG_DEBUG("Same Direction");
        return true;
    }
    NS_LOG_DEBUG("NON same Direction");
    return false;
}

template <typename T> int V2vNovelAlgorithmClient::sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

bool
V2vNovelAlgorithmClient::IsStable(Vector otherVelocity){

    uint32_t size = m_neighborMap.size ();
    if(m_neighborMap.size () == 0){
        size = 1;
    }

    Vector v;
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it) {
        uint64_t key = it->first;
        NS_ASSERT(key != m_currentInfo.id);

        V2vClusterSap::NovelNeighborInfo value = it->second;
        v.x += value.velocity.x;
        v.y += value.velocity.y;
        v.z += value.velocity.z;
    }
    v.x = v.x/size;
    v.y = v.y/size;
    v.z = v.z/size;

    //!< Find standard deviation of velocity
    Vector vs;
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it) {
        uint64_t key = it->first;
        NS_ASSERT(key != m_currentInfo.id);

        V2vClusterSap::NovelNeighborInfo value = it->second;
        vs.x += pow((value.velocity.x - v.x), 2.0);
        vs.y += pow((value.velocity.y - v.y), 2.0);
        vs.z += pow((value.velocity.z - v.z), 2.0);
    }
    vs.x = vs.x/size;
    vs.y = vs.y/size;
    vs.z = vs.z/size;

    Vector thres;
    if(vs.x != 0){
        thres.x = (pow (((m_currentInfo.velocity.x - otherVelocity.x) - v.x), 2.0) / (2*pow (vs.x, 2.0)));
    }
    else{
        thres.x = (pow (((m_currentInfo.velocity.x - otherVelocity.x) - v.x), 2.0) / 1);
    }

    if(vs.y != 0){
        thres.y = (pow (((m_currentInfo.velocity.y - otherVelocity.y) - v.y), 2.0) / (2*pow (vs.y, 2.0)));
    }
    else{
        thres.y = (pow (((m_currentInfo.velocity.y - otherVelocity.y) - v.y), 2.0) / 1);
    }

    if(vs.z != 0){
        thres.z = (pow (((m_currentInfo.velocity.z - otherVelocity.z) - v.z), 2.0) / (2*pow (vs.z, 2.0)));
    }
    else{
        thres.z = (pow (((m_currentInfo.velocity.z - otherVelocity.z) - v.z), 2.0) / 1);
    }

    double t = thres.x + thres.y + thres.z;
    double r = CalculateDistance (m_currentInfo.velocity, otherVelocity);
    if( r > t){
        return false;
    }
    return true;
}

bool
V2vNovelAlgorithmClient::IsSlowestVehicle (void){
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_stableNeighborMap.begin(); it != m_stableNeighborMap.end(); ++it) {
        V2vClusterSap::NovelNeighborInfo value = it->second;
        if(value.tempClusterId == 0){
            if(IsLowerThan(value.velocity, m_currentInfo.velocity)){
                return false;
            }
        }
    }
    NS_LOG_DEBUG("Node:" << m_currentInfo.id << " is the slowest in range.");
    return true;
}

bool
V2vNovelAlgorithmClient::IsLowerThan(Vector v1, Vector v2){
    if(fabs(v1.x + v1.y + v1.z) < fabs(v2.x + v2.y + v2.z)){
        return true;
    }
    return false;
}

double
V2vNovelAlgorithmClient::SuitabilityCheck (void){

    double size = m_stableNeighborMap.size();

    //!< If no neighbours found, return standard TDMA vehicle's window
    if(m_stableNeighborMap.size() == 0){
        size = 1;
    }

    Vector p,v;
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_stableNeighborMap.begin(); it != m_stableNeighborMap.end(); ++it) {
        uint64_t key = it->first;
        NS_ASSERT(key != m_currentInfo.id);

        V2vClusterSap::NovelNeighborInfo value = it->second;
        p.x += value.position.x;
        p.y += value.position.y;
        p.z += value.position.z;

        v.x += value.velocity.x;
        v.y += value.velocity.y;
        v.z += value.velocity.z;
    }
    p.x = p.x/size;
    p.y = p.y/size;
    p.z = p.z/size;
    v.x = v.x/size;
    v.y = v.y/size;
    v.z = v.z/size;

    NS_LOG_DEBUG("Mean p.x = " << p.x << " - Mean p.y = " << p.y << " - Mean p.z = " << p.z);
    NS_LOG_DEBUG("Mean v.x = " << v.x << " - Mean v.y = " << v.y << " - Mean p.z = " << p.z);


    //!< Find standard deviation of position and velocity
    Vector ps,vs;
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_stableNeighborMap.begin(); it != m_stableNeighborMap.end(); ++it) {
        uint64_t key = it->first;
        NS_ASSERT(key != m_currentInfo.id);

        V2vClusterSap::NovelNeighborInfo value = it->second;
        ps.x += pow((value.position.x - p.x), 2.0);
        ps.y += pow((value.position.y - p.y), 2.0);
        ps.z += pow((value.position.z - p.z), 2.0);
        vs.x += pow((value.velocity.x - v.x), 2.0);
        vs.y += pow((value.velocity.y - v.y), 2.0);
        vs.z += pow((value.velocity.z - v.z), 2.0);
    }
    ps.x = ps.x/size;
    ps.y = ps.y/size;
    ps.z = ps.z/size;
    vs.x = vs.x/size;
    vs.y = vs.y/size;
    vs.z = vs.z/size;

    NS_LOG_DEBUG("ps.x = " << ps.x << " - ps.y = " << ps.y << " - ps.z = " << ps.z);
    NS_LOG_DEBUG("vs.x = " << vs.x << " - vs.y = " << vs.y << " - vs.z = " << vs.z);

    /// Avoid division with zero if vehicle moves horizontally or vertiacally
    Vector pNorm;
    if(ps.x == 0){
        pNorm.x = (m_currentInfo.position.x - p.x)/1;
    }
    else{
        pNorm.x = (m_currentInfo.position.x - p.x)/ps.x;
    }

    if(ps.y == 0){
        pNorm.y = (m_currentInfo.position.y - p.y)/1;
    }
    else{
        pNorm.y = (m_currentInfo.position.y - p.y)/ps.y;
    }

    if(ps.z == 0){
        pNorm.z = (m_currentInfo.position.z - p.z)/1;
    }
    else{
        pNorm.z = (m_currentInfo.position.z - p.z)/ps.z;
    }


    Vector vNorm;
    if(vs.x == 0){
        vNorm.x = (m_currentInfo.velocity.x - v.x)/1;
    }
    else{
        vNorm.x = (m_currentInfo.velocity.x - v.x)/vs.x;
    }

    if(vs.y == 0){
        vNorm.y = (m_currentInfo.velocity.y - v.y)/1;
    }
    else{
        vNorm.y = (m_currentInfo.velocity.y - v.y)/vs.y;
    }

    if(vs.z == 0){
        vNorm.z = (m_currentInfo.velocity.z - v.z)/1;
    }
    else{
        vNorm.z = (m_currentInfo.velocity.z - v.z)/vs.z;
    }
    NS_LOG_DEBUG("pNorm.x:" << pNorm.x << " - vNorm.x:" << vNorm.x);
    NS_LOG_DEBUG("pNorm.y:" << pNorm.y << " - vNorm.y:" << vNorm.y);
    NS_LOG_DEBUG("pNorm.z:" << pNorm.z << " - vNorm.z:" << vNorm.z);


    double w = fabs(pNorm.x) + fabs(pNorm.y) + fabs(pNorm.z) + fabs(vNorm.x) + fabs(vNorm.y) + fabs(vNorm.z);
    NS_LOG_DEBUG("w = " << w << " - u = " << (double)size * exp((-m_clusterTimeMetric)* w) << " - size:" << size);

    return (double)size * exp((-m_clusterTimeMetric)* w);
}

uint64_t
V2vNovelAlgorithmClient::FindStableClusterHead (void){
    uint64_t id = 0;
    double r = 80;                          //!< transmition range
    double rt = 0.0;                        //!< Suitability metric for CH  selection
    double boundary = 0.0;
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator itSearch = m_stableNeighborMap.begin(); itSearch != m_stableNeighborMap.end(); ++itSearch){
        V2vClusterSap::NovelNeighborInfo node = itSearch->second;
        if(node.degree == V2vClusterSap::CH){
            if( ((m_currentInfo.position.x < node.position.x) && (m_currentInfo.velocity.x > 0) && fabs(m_currentInfo.velocity.x) < fabs(node.velocity.x))
                    || ((m_currentInfo.position.x < node.position.x) && (m_currentInfo.velocity.x < 0) && fabs(m_currentInfo.velocity.x) > fabs(node.velocity.x)) ){
                rt = (r-fabs(m_currentInfo.position.x - node.position.x)) / (fabs(m_currentInfo.velocity.x-node.velocity.x));
                NS_LOG_DEBUG ("Nodes increasingly removed  - RT:" << rt << "current Node:" << m_currentInfo.id << " - with node:" << node.id);
            }
            if( ((m_currentInfo.position.x < node.position.x) && (m_currentInfo.velocity.x > 0) && fabs(m_currentInfo.velocity.x) > fabs(node.velocity.x))
                    || ((m_currentInfo.position.x < node.position.x) && (m_currentInfo.velocity.x < 0) && fabs(m_currentInfo.velocity.x) < fabs(node.velocity.x)) ){
                rt = (r+fabs(m_currentInfo.position.x - node.position.x)) / (fabs(m_currentInfo.velocity.x-node.velocity.x));
                NS_LOG_DEBUG("Nodes increasingly approaching - RT:" << rt << "current Node:" << m_currentInfo.id << " - with node:" << node.id);
            }

            if( ((m_currentInfo.position.x > node.position.x) && (m_currentInfo.velocity.x > 0) && fabs(m_currentInfo.velocity.x) > fabs(node.velocity.x))
                    || ((m_currentInfo.position.x > node.position.x) && (m_currentInfo.velocity.x < 0) && fabs(m_currentInfo.velocity.x) < fabs(node.velocity.x)) ){
                rt = (r-fabs(m_currentInfo.position.x - node.position.x)) / (fabs(m_currentInfo.velocity.x-node.velocity.x));
                NS_LOG_DEBUG("Nodes increasingly removed - RT:" << rt << "current Node:" << m_currentInfo.id << " - with node:" << node.id);
            }
            if( ((m_currentInfo.position.x > node.position.x) && (m_currentInfo.velocity.x > 0) && fabs(m_currentInfo.velocity.x) < fabs(node.velocity.x))
                    || ((m_currentInfo.position.x > node.position.x) && (m_currentInfo.velocity.x < 0) && fabs(m_currentInfo.velocity.x) > fabs(node.velocity.x)) ){
                rt = (r+fabs(m_currentInfo.position.x - node.position.x)) / (fabs(m_currentInfo.velocity.x-node.velocity.x));
                NS_LOG_DEBUG("Nodes increasingly approaching - RT:" << rt << "current Node:" << m_currentInfo.id << " - with node:" << node.id);
            }

            if(rt > boundary){
                id = itSearch->first;
                boundary = rt;
            }
        }
    }
    NS_LOG_DEBUG ("Returned Id is: " << id << " - with Remaining Time(RT):" << boundary);
    return id;
}

void
V2vNovelAlgorithmClient::ChangeState(V2vClusterSap::NovelNodeState currentState){
    switch (currentState) {
    case V2vClusterSap::UNDEFINED:
        m_nodeState = V2vClusterSap::INITIAL;
        break;
    case V2vClusterSap::INITIAL:
        m_nodeState = V2vClusterSap::COV;
        break;
    case V2vClusterSap::COV:
        m_nodeState = V2vClusterSap::FORMATION;
        break;
    case V2vClusterSap::FORMATION:
        m_nodeState = V2vClusterSap::UPDATE;
        break;
    default:
        m_nodeState = V2vClusterSap::UPDATE;
        break;
    }
    NS_LOG_DEBUG("Node: " << m_currentInfo.id << " enters in state: " << ToString (m_nodeState) << " at Time: " << Simulator::Now ().GetSeconds ());
}

void
V2vNovelAlgorithmClient::StatusReport (void){
    NS_LOG_UNCOND("\n\n-----------------------------------------------------------------------------");
    NS_LOG_UNCOND ("[StatusReport] => At time " << Simulator::Now ().GetSeconds ()
        << "s node ["<< m_currentInfo.id << "] - " << ToString (m_currentInfo.degree) << " is in state: "
        << ToString (m_nodeState)
        << " and in Cluster: " << m_currentInfo.clusterId << " having: " << m_currentInfo.chMembers << " cluster Members"
        << " \n => position: " << m_currentInfo.position
        << " - Velocity: " << m_currentInfo.velocity
        << " - Direction: " << m_currentInfo.direction
        << " Last message sent:" << TimeStep (m_currentInfo.ts).GetSeconds ());

    NS_LOG_UNCOND("----------------------------  Neighbour List  ---------------------------------");
    NS_LOG_UNCOND("m_neighborMap size:" << m_neighborMap.size ());
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it){
        V2vClusterSap::NovelNeighborInfo node = it->second;
        NS_LOG_UNCOND(" Id:" << node.id
                << " ClusterId:" << node.clusterId
                << " TempClusterId:" << node.tempClusterId
                << " Cluster Members:" << node.chMembers
                << " Position:" << node.position
                << " Velocity:" << node.velocity
                << " Direction:" << node.direction
                << " Last Message Received:" << TimeStep(node.ts).GetSeconds());
    }

    NS_LOG_UNCOND("----------------------------  Stable Neighbour List  ---------------------------------");
    NS_LOG_UNCOND("m_stableNeighborMap size:" << m_stableNeighborMap.size ());
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_stableNeighborMap.begin(); it != m_stableNeighborMap.end(); ++it){
        V2vClusterSap::NovelNeighborInfo node = it->second;
        NS_LOG_UNCOND(" Id:" << node.id
                << " ClusterId:" << node.clusterId
                << " TempClusterId:" << node.tempClusterId
                << " Cluster Members:" << node.chMembers
                << " Position:" << node.position
                << " Velocity:" << node.velocity
                << " Direction:" << node.direction
                << " Last Message Received:" << TimeStep(node.ts).GetSeconds());
    }

    NS_LOG_UNCOND("----------------------------  Cluster List  ---------------------------------");
    NS_LOG_UNCOND("m_clusterMap size:" << m_clusterMap.size ());
    for(std::map<uint64_t, V2vClusterSap::NovelNeighborInfo>::iterator it = m_clusterMap.begin(); it != m_clusterMap.end(); ++it){
        V2vClusterSap::NovelNeighborInfo node = it->second;
        NS_LOG_UNCOND(" Id:" << node.id
                << " ClusterId:" << node.clusterId
                << " TempClusterId:" << node.tempClusterId
                << " Cluster Members:" << node.chMembers
                << " Position:" << node.position
                << " Velocity:" << node.velocity
                << " Direction:" << node.direction
                << " Last Message Received:" << TimeStep(node.ts).GetSeconds());
    }
    NS_LOG_UNCOND("-----------------------------------------------------------------------------\n");

}

} // Namespace ns3
