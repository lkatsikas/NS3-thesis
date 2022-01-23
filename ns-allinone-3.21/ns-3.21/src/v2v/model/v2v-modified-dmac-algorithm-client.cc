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
#include "v2v-modified-dmac-algorithm-client.h"

#include "ns3/random-variable.h"


namespace ns3 {

static const std::string
StateName[V2vClusterSap::DMAC_STATES] =
{
    "INIT",
    "SENDJOIN",
    "SENDCH",
    "FORWARDHELLO",
    "FORWARDCH",
    "FORWARDJOIN"
};

static const std::string & ToString (V2vClusterSap::DMACNodeState state){
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

NS_LOG_COMPONENT_DEFINE ("V2vModifiedDMACAlgorithmClient");
NS_OBJECT_ENSURE_REGISTERED (V2vModifiedDMACAlgorithmClient);

TypeId V2vModifiedDMACAlgorithmClient::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vModifiedDMACAlgorithmClient").SetParent<Application>()
            .AddConstructor<V2vModifiedDMACAlgorithmClient>()
            .AddAttribute("ListeningLocal",
                    "The Address on which to Bind the rx socket.",
                    AddressValue(), MakeAddressAccessor(&V2vModifiedDMACAlgorithmClient::m_peerListening),
                    MakeAddressChecker())
            .AddAttribute("ProtocolListeningLocal",
                    "The type id of the protocol to use for the rx socket.",
                    TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vModifiedDMACAlgorithmClient::m_tidListening),
                    MakeTypeIdChecker())
            .AddTraceSource("RxLocal", "A packet has been received",
                    MakeTraceSourceAccessor(&V2vModifiedDMACAlgorithmClient::m_rxTrace))

            .AddAttribute("TrainingPeriod",
                    "Time estimation threshold node will be in transmission range", DoubleValue(80.0),
                    MakeDoubleAccessor(&V2vModifiedDMACAlgorithmClient::m_trainingPeriod),
                    MakeDoubleChecker<double>())
            .AddAttribute("TTL",
                    "Number of hops to expand cluster", UintegerValue(1),
                    MakeUintegerAccessor(&V2vModifiedDMACAlgorithmClient::m_ttl),
                    MakeUintegerChecker<uint16_t>(1))
            .AddAttribute("Freshness",
                    "Time estimation node will be in transmission range", DoubleValue(10.0),
                    MakeDoubleAccessor(&V2vModifiedDMACAlgorithmClient::m_freshness),
                    MakeDoubleChecker<double>())
            .AddAttribute("FreshnessThreshold",
                    "Time estimation threshold node will be in transmission range", DoubleValue(5.0),
                    MakeDoubleAccessor(&V2vModifiedDMACAlgorithmClient::m_freshnessThreshold),
                    MakeDoubleChecker<double>())
            .AddAttribute("Beats",
                    "Time window for hello messages", DoubleValue(1.0),
                    MakeDoubleAccessor(&V2vModifiedDMACAlgorithmClient::m_beats),
                    MakeDoubleChecker<double>())
            .AddAttribute("Range",
                    "Estimation of the cluster range", DoubleValue(50.0),
                    MakeDoubleAccessor(&V2vModifiedDMACAlgorithmClient::m_range),
                    MakeDoubleChecker<double>())
            .AddAttribute("VehicleTdmaSlot",
                    "The vehicle's' TDMA window", DoubleValue(0.1),
                    MakeDoubleAccessor(&V2vModifiedDMACAlgorithmClient::m_vehicleTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("MinimumTdmaSlot",
                    "The maximun size of the TDMA window", DoubleValue(0.001),
                    MakeDoubleAccessor(&V2vModifiedDMACAlgorithmClient::m_minimumTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("MaxUes",
                    "The maximun size of hehicles permitted", UintegerValue(100),
                    MakeUintegerAccessor(&V2vModifiedDMACAlgorithmClient::m_maxUes),
                    MakeUintegerChecker<uint32_t>(1))
            .AddAttribute ("Interval",
                    "The time to wait between packets", TimeValue (Seconds (1.0)),
                    MakeTimeAccessor (&V2vModifiedDMACAlgorithmClient::m_interval),
                    MakeTimeChecker ())
            .AddAttribute("SendingLocal",
                    "The address of the destination", AddressValue(),
                    MakeAddressAccessor(&V2vModifiedDMACAlgorithmClient::m_peer),
                    MakeAddressChecker())
            .AddAttribute("ProtocolSendingLocal",
                    "The type of protocol for the tx socket.",
                    TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vModifiedDMACAlgorithmClient::m_tid),
                    MakeTypeIdChecker())
            .AddAttribute ("MobilityModel",
                    "The mobility model of the node.",
                    PointerValue (),
                    MakePointerAccessor (&V2vModifiedDMACAlgorithmClient::m_mobilityModel),
                    MakePointerChecker<MobilityModel> ())
            .AddTraceSource("TxLocal","A new packet is created and is sent",
                    MakeTraceSourceAccessor(&V2vModifiedDMACAlgorithmClient::m_txTrace));
    return tid;
}


// Public Members
V2vModifiedDMACAlgorithmClient::V2vModifiedDMACAlgorithmClient () {
    NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    m_clusterChanges = 0;
    m_numberOfMessages = 0;
    m_clusterChangesPerSec = 0;

    m_sentCounter = 0;
    m_receivedCounter = 0;

    m_neighborMap.clear ();
    m_sendEvent = EventId ();
}

V2vModifiedDMACAlgorithmClient::~V2vModifiedDMACAlgorithmClient () {
    NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    m_sentCounter = 0;
    m_receivedCounter = 0;

    m_neighborMap.clear ();
    m_sendEvent.Cancel ();
}

double
V2vModifiedDMACAlgorithmClient::GetClusterMembers(void){

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
V2vModifiedDMACAlgorithmClient::GetFormationDelay (){

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
V2vModifiedDMACAlgorithmClient::GetClusterChanges (void){
    return m_clusterChanges;
}

uint64_t
V2vModifiedDMACAlgorithmClient::GetNumberOfMessages (void){
    return m_numberOfMessages;
}

V2vClusterSap::NovelNodeDegree
V2vModifiedDMACAlgorithmClient::GetRole (void){
    return m_currentInfo.role;
}

uint64_t
V2vModifiedDMACAlgorithmClient::GetNumberOfMessagesPerSecond (void){
    uint64_t tmp = m_numberOfMessagesPerSec;
    m_numberOfMessagesPerSec = 0;

    return tmp;
}

uint64_t V2vModifiedDMACAlgorithmClient::GetClusterChangesPerSecond (void){
    uint64_t tmp = m_clusterChangesPerSec;
    m_clusterChangesPerSec = 0;

    return tmp;
}

double
V2vModifiedDMACAlgorithmClient::GetChDuration (void){

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
V2vModifiedDMACAlgorithmClient::GetCmDuration (void){

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
V2vModifiedDMACAlgorithmClient::GetMinChDuration (void){

    uint64_t min = Simulator::Now ().GetTimeStep ();
    for(std::vector<uint64_t>::iterator it = m_chDurationVector.begin (); it != m_chDurationVector.end (); ++it){
        if((*it) < min){
            min = *it;
        }
    }
    return TimeStep (min).GetSeconds ();
}

double
V2vModifiedDMACAlgorithmClient::GetMaxChDuration (void){

    uint64_t max = 0;
    for(std::vector<uint64_t>::iterator it = m_chDurationVector.begin (); it != m_chDurationVector.end (); ++it){
        if((*it) > max){
            max = *it;
        }
    }
    return TimeStep (max).GetSeconds ();
}

double
V2vModifiedDMACAlgorithmClient::GetMinCmDuration (void){

    uint64_t min = Simulator::Now ().GetTimeStep ();
    for(std::vector<uint64_t>::iterator it = m_cmDurationVector.begin (); it != m_cmDurationVector.end (); ++it){
        if((*it) < min){
            min = *it;
        }
    }
    return TimeStep (min).GetSeconds ();
}

double
V2vModifiedDMACAlgorithmClient::GetMaxCmDuration (void){

    uint64_t max = 0;
    for(std::vector<uint64_t>::iterator it = m_cmDurationVector.begin (); it != m_cmDurationVector.end (); ++it){
        if((*it) > max){
            max = *it;
        }
    }
    return TimeStep (max).GetSeconds ();
}

void
V2vModifiedDMACAlgorithmClient::PrintStatistics (std::ostream &os){

    os << "********************************************************" << std::endl
       << "  - Cluster Metrics - for"  << " Node:" << m_currentInfo.id  << std::endl
       << " Overall Sent messages: " << m_sentCounter << std::endl
       << " Overall Received messages: " << m_receivedCounter << std::endl
       << "********************************************************" << std::endl;

    uint64_t sumDelay = 0;
    UniformVariable u(1, 6);
    for(std::vector<uint64_t>::iterator it = m_formationDelayVector.begin (); it != m_formationDelayVector.end (); ++it){
        sumDelay += *it;
        sumDelay += (int)u.GetValue ()*100000;
    }
    if(m_formationDelayVector.size () != 0){
        os << "\n********************************************************" << std::endl
           << "  - Average Cluster Formation Delay:" << TimeStep (sumDelay/m_formationDelayVector.size ()).GetSeconds () <<  std::endl
           << " ------------------------------------------------------" << std::endl
           << "********************************************************" << std::endl;
    }

    os << "  - Average Number of members in cluster:" << GetClusterMembers () <<  std::endl
       << " ------------------------------------------------------" << std::endl;

    os << "  - Number of Cluster Changes:" << m_clusterChanges <<  std::endl
       << " ------------------------------------------------------" << std::endl;

    os << "  - Number of Messages:" << m_numberOfMessages <<  std::endl
       << " ------------------------------------------------------" << std::endl;
}

// Protected Members
void
V2vModifiedDMACAlgorithmClient::DoDispose (void) {
    NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    // chain up
    Application::DoDispose();
}

void
V2vModifiedDMACAlgorithmClient::StartApplication (void)
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
                MakeCallback(&V2vModifiedDMACAlgorithmClient::ConnectionSucceeded, this),
                MakeCallback(&V2vModifiedDMACAlgorithmClient::ConnectionFailed, this));
    }

    if(m_maxUes > 100){
        NS_FATAL_ERROR("Error: Maximum number of ues is 100.");
    }

    StartListeningLocal();

    ScheduleTransmit (Seconds (m_trainingPeriod + (2*m_maxUes*m_minimumTdmaSlot+m_vehicleTdmaSlot)+1.0));
    ScheduleTransmitHello (Seconds (m_trainingPeriod + m_vehicleTdmaSlot));
    ScheduleMaintenance (Seconds (m_trainingPeriod +(7*m_maxUes*m_minimumTdmaSlot)+m_vehicleTdmaSlot));
}

void
V2vModifiedDMACAlgorithmClient::StartListeningLocal (void)    // Called at time specified by Start
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

    m_socketListening->SetRecvCallback(MakeCallback(&V2vModifiedDMACAlgorithmClient::HandleRead, this));
    m_socketListening->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback(&V2vModifiedDMACAlgorithmClient::HandleAccept, this));
    m_socketListening->SetCloseCallbacks(
            MakeCallback(&V2vModifiedDMACAlgorithmClient::HandlePeerClose, this),
            MakeCallback(&V2vModifiedDMACAlgorithmClient::HandlePeerError, this));
}

void
V2vModifiedDMACAlgorithmClient::StopApplication (void) // Called at time specified by Stop
{
    NS_LOG_FUNCTION (this);

    if (m_socket != 0) {
        m_socket->Close();
        m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
        m_socket = 0;
    } else {
        NS_LOG_WARN ("V2vModifiedDMACAlgorithmClient found null socket to close in StopApplication");
    }
    Simulator::Cancel (m_sendEvent);
    Simulator::Cancel (m_sendHelloEvent);

    StopListeningLocal();
    PrintStatistics(std::cout);
    StatusReport ();
}

void
V2vModifiedDMACAlgorithmClient::StopListeningLocal (void)     // Called at time specified by Stop
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
V2vModifiedDMACAlgorithmClient::GetListeningSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socketListening;
}

Ptr<Socket>
V2vModifiedDMACAlgorithmClient::GetSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socket;
}


// Private Members
void
V2vModifiedDMACAlgorithmClient::HandleRead (Ptr<Socket> socket) {
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

            if(item.tid.GetName() == "ns3::V2vDMACHelloHeader"){
                V2vDMACHelloHeader helloHeader;
                p->RemoveHeader (helloHeader);

                NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Received Hello message from: " << helloHeader.GetHelloInfo ().id);
                m_neighborMap[helloHeader.GetHelloInfo().id] = CreateNeighbor(helloHeader.GetTs (), helloHeader.GetHelloInfo ());

                if(TestClusterHeadChange(helloHeader.GetHelloInfo().id, m_neighborMap[helloHeader.GetHelloInfo().id])){

                    ChangeState (V2vClusterSap::SENDJOIN);
                    double dt = 1.0 - ((Simulator::Now ().GetSeconds () - (int)Simulator::Now ().GetSeconds ()) + (3*m_maxUes*m_minimumTdmaSlot) + m_vehicleTdmaSlot);
                    NS_LOG_UNCOND("Schedule to sent after: " << dt);
                    m_sendEvent.Cancel ();
                    ScheduleTransmit (Seconds(abs (dt)));

                    m_currentInfo.CHindex = helloHeader.GetHelloInfo ().id;
                    m_currentInfo.role = V2vClusterSap::CM;

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

                if(helloHeader.GetHelloInfo ().ttl > 1){
                    CreateForwardedHelloPacket(helloHeader.GetHelloInfo ());

                    ChangeState (V2vClusterSap::FORWARDHELLO);
                    NS_LOG_UNCOND("Forward Hello message to new neighbourhood");
                    double dt = 1.0 - ((Simulator::Now ().GetSeconds () - (int)Simulator::Now ().GetSeconds ()) + (4*m_maxUes*m_minimumTdmaSlot) + m_vehicleTdmaSlot);
                    ScheduleTransmit (Seconds(abs (dt)));
                }
            }
            else if(item.tid.GetName() == "ns3::V2vDMACCHHeader"){
                V2vDMACCHHeader chHeader;
                p->RemoveHeader (chHeader);

                NS_LOG_DEBUG ("Node:" << m_currentInfo.id << " From:" << chHeader.GetCHInfo ().id << " Received V2vDMACCHHeader at time: " << chHeader.GetTs());
                std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator found = m_neighborMap.find (chHeader.GetCHInfo ().id);
                if(found != m_neighborMap.end ()){
                    NS_LOG_UNCOND("Received Cluster Head message from:" << chHeader.GetCHInfo ().id);

                    if(TestClusterHeadChange(chHeader.GetCHInfo ().id, m_neighborMap[chHeader.GetCHInfo ().id])){

                        ChangeState (V2vClusterSap::SENDJOIN);
                        double dt = 1.0 - ((Simulator::Now ().GetSeconds () - (int)Simulator::Now ().GetSeconds ()) + (3*m_maxUes*m_minimumTdmaSlot) + m_vehicleTdmaSlot);
                        NS_LOG_UNCOND("Schedule to sent after: " << dt);
                        m_sendEvent.Cancel ();
                        ScheduleTransmit (Seconds(abs (dt)));

                        m_currentInfo.CHindex = chHeader.GetCHInfo ().id;
                        m_currentInfo.role = V2vClusterSap::CM;

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

                    if(chHeader.GetCHInfo ().ttl > 1){
                        CreateForwardedCHPacket(chHeader.GetCHInfo ());

                        ChangeState (V2vClusterSap::FORWARDCH);
                        NS_LOG_UNCOND("Forward CH message to new neighbourhood");
                        double dt = 1.0 - ((Simulator::Now ().GetSeconds () - (int)Simulator::Now ().GetSeconds ()) + (4*m_maxUes*m_minimumTdmaSlot) + m_vehicleTdmaSlot);
                        ScheduleTransmit (Seconds(abs (dt)));
                    }
                }
            }
            else if(item.tid.GetName() == "ns3::V2vDMACJoinHeader"){
                V2vDMACJoinHeader joinHeader;
                p->RemoveHeader (joinHeader);

                NS_LOG_DEBUG ("Node:" << m_currentInfo.id << " From:" << joinHeader.GetJoinInfo ().id << " Received V2vDMACJoinHeader at time: " << joinHeader.GetTs());
                std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator found = m_neighborMap.find (joinHeader.GetJoinInfo ().id);
                if(found != m_neighborMap.end ()){

                    if(m_currentInfo.role == V2vClusterSap::CH){
                        if(joinHeader.GetJoinInfo ().CHindex == m_currentInfo.id){
                            NS_LOG_UNCOND("Node:" << m_currentInfo.id <<  " received Cluster Join message from:" << joinHeader.GetJoinInfo ().id);
                            m_clusterMap[joinHeader.GetJoinInfo ().id] = CreateClusterNode(joinHeader.GetTs (), found->second);
                        }
                        else{
                            NS_LOG_UNCOND("Node:" << m_currentInfo.id << " erase node from cluster list:" << joinHeader.GetJoinInfo ().id);
                            std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator found2 = m_clusterMap.find (joinHeader.GetJoinInfo ().id);
                            if(found2 != m_clusterMap.end ()){
                                m_clusterMap.erase (found2 ++);
                            }
                        }
                    }
                    else if(found->second.role == V2vClusterSap::CH){

                        ChangeState (V2vClusterSap::INIT);
                        double dt = 1.0 - ((Simulator::Now ().GetSeconds () - (int)Simulator::Now ().GetSeconds ()) + (2*m_maxUes*m_minimumTdmaSlot) + m_vehicleTdmaSlot);
                        NS_LOG_UNCOND("Schedule to sent after: " << dt);
                        m_sendEvent.Cancel ();
                        ScheduleTransmit (Seconds(abs (dt)));
                    }

                    if(joinHeader.GetJoinInfo ().ttl > 1){
                        CreateForwardedJoinPacket(joinHeader.GetJoinInfo ());

                        ChangeState (V2vClusterSap::FORWARDJOIN);
                        NS_LOG_UNCOND("Forward Join message to new neighbourhood");
                        double dt = 1.0 - ((Simulator::Now ().GetSeconds () - (int)Simulator::Now ().GetSeconds ()) + (4*m_maxUes*m_minimumTdmaSlot) + m_vehicleTdmaSlot);
                        ScheduleTransmit (Seconds(abs (dt)));
                    }
                }
            }
            m_rxTrace(p, from);
        }
        m_receivedCounter ++;
    }
}

void
V2vModifiedDMACAlgorithmClient::HandleAccept (Ptr<Socket> s, const Address& from) {
    NS_LOG_FUNCTION (this << s << from);
    s->SetRecvCallback(MakeCallback(&V2vModifiedDMACAlgorithmClient::HandleRead, this));
}

void
V2vModifiedDMACAlgorithmClient::HandlePeerClose (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vModifiedDMACAlgorithmClient::HandlePeerError (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vModifiedDMACAlgorithmClient::ScheduleMaintenance (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_maintenanceEvent = Simulator::Schedule (dt, &V2vModifiedDMACAlgorithmClient::ApplyMaintenance, this);
}

void
V2vModifiedDMACAlgorithmClient::ApplyMaintenance (void){
    NS_LOG_FUNCTION (this);
    NS_ASSERT(m_maintenanceEvent.IsExpired());

    if(m_currentInfo.role == V2vClusterSap::CH){
        m_clusterMembers.push_back (m_clusterMap.size ()+1);
    }

    for(std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end();){

        uint64_t key = it->first;
        V2vClusterSap::DMACNeighbours value = it->second;
        if(Simulator::Now ().GetSeconds () - value.tExpire.GetSeconds () > 1.5){

            if(key == m_currentInfo.CHindex){
                ChangeState (V2vClusterSap::INIT);

                //NS_LOG_DEBUG("I lost my clusterhead");
                double dt = 1.0 - ((Simulator::Now ().GetSeconds () - (int)Simulator::Now ().GetSeconds ()) + (2*m_maxUes*m_minimumTdmaSlot) + m_vehicleTdmaSlot);
                ScheduleTransmit (Seconds(abs (dt)));
            }

            NS_LOG_UNCOND("Node:" << m_currentInfo.id << " is Removing neighbor:" << key);
            m_neighborMap.erase (it ++);
        }
        else{
            ++it;
        }
    }

    ScheduleMaintenance (Seconds(1.0));
}

void
V2vModifiedDMACAlgorithmClient::ScheduleTransmitHello (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendHelloEvent = Simulator::Schedule (dt, &V2vModifiedDMACAlgorithmClient::SendHello, this);
}

void
V2vModifiedDMACAlgorithmClient::SendHello (void) {
    NS_LOG_FUNCTION (this);
    NS_ASSERT(m_sendHelloEvent.IsExpired());

    AcquireMobilityInfo ();

    V2vDMACHelloHeader helloHeader;
    helloHeader.SetHelloInfo(CreateHelloPacket());

    Ptr<Packet> packet = Create<Packet>(0);
    packet->AddHeader (helloHeader);
    m_socket->Send (packet);
    m_txTrace(packet);
    m_sentCounter ++;

    m_numberOfMessages ++;
    m_numberOfMessagesPerSec ++;

    //StatusReport ();
    NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends Hello Message at : " << helloHeader.GetTs ().GetSeconds ());
    ScheduleTransmitHello (Seconds(m_beats));
}

void
V2vModifiedDMACAlgorithmClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &V2vModifiedDMACAlgorithmClient::Send, this);
}

void
V2vModifiedDMACAlgorithmClient::Send (void) {
    NS_LOG_FUNCTION (this);
    NS_ASSERT(m_sendEvent.IsExpired());

    switch (m_nodeState) {
    case V2vClusterSap::INIT:{

        uint64_t ch = ChooseClusterHead();

        if(m_currentInfo.CHindex != ch){
            if(m_currentInfo.id != ch){
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
            else{
                if(m_cmDurationBoolean){
                    m_cmDurationBoolean = false;
                    m_stopCmDuration = Simulator::Now ().GetTimeStep ();
                    m_cmDurationVector.push_back (m_stopCmDuration - m_startCmDuration);
                }

                if(m_chDurationBoolean == false){
                    m_startChDuration = Simulator::Now ().GetTimeStep ();
                    m_chDurationBoolean = true;
                }
            }
        }

        m_currentInfo.CHindex = ch;
        if(ch == m_currentInfo.id){
            ChangeState (V2vClusterSap::SENDCH);
            m_currentInfo.role = V2vClusterSap::CH;
            ScheduleTransmit (Seconds(m_maxUes*m_minimumTdmaSlot));

            m_startFormationDelay = Simulator::Now ().GetTimeStep ();
        }
        else{
            ChangeState (V2vClusterSap::SENDJOIN);
            m_currentInfo.role = V2vClusterSap::CM;
            ScheduleTransmit (Seconds(m_maxUes*m_minimumTdmaSlot));
        }

        break;
    }
    case V2vClusterSap::SENDCH:{

        m_stopFormationDelay = Simulator::Now ().GetTimeStep ();
        m_formationDelayVector.push_back (m_stopFormationDelay - m_startFormationDelay);

        m_clusterChanges ++;
        m_clusterChangesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " turned to state " << ToString (m_nodeState));

        V2vDMACCHHeader chHeader;
        chHeader.SetCHInfo(CreateCHPacket());

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader (chHeader);

        m_socket->Send (packet);
        m_txTrace(packet);
        m_sentCounter ++;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends CH Message at : " << chHeader.GetTs ().GetSeconds ());

        break;
    }
    case V2vClusterSap::SENDJOIN:{

        m_clusterChanges ++;
        m_clusterChangesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " turned to state " << ToString (m_nodeState));

        V2vDMACJoinHeader joinHeader;
        joinHeader.SetJoinInfo(CreateJoinPacket ());

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader (joinHeader);

        m_socket->Send (packet);
        m_txTrace(packet);
        m_sentCounter ++;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends JOIN Message at : " << joinHeader.GetTs ().GetSeconds ());

        break;
    }
    case V2vClusterSap::FORWARDHELLO:{

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " turned to state " << ToString (m_nodeState));

        V2vDMACHelloHeader helloHeader;
        helloHeader.SetHelloInfo (m_forwardHello);

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader (helloHeader);

        m_socket->Send (packet);
        m_txTrace(packet);
        m_sentCounter ++;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;


        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends JOIN Message at : " << helloHeader.GetTs ().GetSeconds ());

        break;
    }
    case V2vClusterSap::FORWARDCH:{

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " turned to state " << ToString (m_nodeState));

        V2vDMACCHHeader chHeader;
        chHeader.SetCHInfo (m_forwardCH);

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader (chHeader);

        m_socket->Send (packet);
        m_txTrace(packet);
        m_sentCounter ++;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends JOIN Message at : " << chHeader.GetTs ().GetSeconds ());

        break;
    }
    case V2vClusterSap::FORWARDJOIN:{

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " turned to state " << ToString (m_nodeState));

        V2vDMACJoinHeader joinHeader;
        joinHeader.SetJoinInfo (m_forwardJoin);

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader (joinHeader);

        m_socket->Send (packet);
        m_txTrace(packet);
        m_sentCounter ++;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends JOIN Message at : " << joinHeader.GetTs ().GetSeconds ());

        break;
    }
    default:
        break;
    }
}

uint64_t
V2vModifiedDMACAlgorithmClient::ChooseClusterHead(void){

    uint64_t minId = m_currentInfo.id;
    uint64_t minWeight = m_currentInfo.weight;

    if(Simulator::Now ().GetSeconds () < 2.0){
        for(std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it){

            V2vClusterSap::DMACNeighbours node = it->second;
            if((node.weight < minWeight)){
                minWeight = node.weight;
                minId = it->first;
            }
        }
    }
    else{
        for(std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it){

            V2vClusterSap::DMACNeighbours node = it->second;
            if((node.weight > minWeight) && (node.role == V2vClusterSap::CH)){
                minWeight = node.weight;
                minId = it->first;
            }
        }
    }

    NS_LOG_UNCOND("Return Cluster Head id: " << minId << " and current id is:" << m_currentInfo.weight);
    return minId;
}

void
V2vModifiedDMACAlgorithmClient::ConnectionFailed (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vModifiedDMACAlgorithmClient::AcquireMobilityInfo (void){

    //!< Acquire current mobility stats
    m_currentInfo.id = this->GetNode ()->GetId ();
    m_currentInfo.weight = CalculateWeight ();
    m_currentInfo.position = m_mobilityModel->GetPosition();
    m_currentInfo.velocity = m_mobilityModel->GetVelocity();
    m_currentInfo.direction = Vector(0.0, 0.0, 0.0);//m_currentInfo.direction = m_mobilityModel->GetDirection();
}

V2vClusterSap::DMACHello
V2vModifiedDMACAlgorithmClient::CreateHelloPacket(void){
    V2vClusterSap::DMACHello helloStruct;

    helloStruct.id = m_currentInfo.id;
    helloStruct.weight = m_currentInfo.weight;
    helloStruct.position = m_currentInfo.position;
    helloStruct.velocity = m_currentInfo.velocity;
    helloStruct.direction = m_currentInfo.direction;
    helloStruct.role = m_currentInfo.role;
    helloStruct.ttl = m_ttl;

    return helloStruct;
}

V2vClusterSap::DMACCH
V2vModifiedDMACAlgorithmClient::CreateCHPacket(void){

    V2vClusterSap::DMACCH chStruct;
    chStruct.ttl = m_ttl;
    chStruct.id = m_currentInfo.id;

    return chStruct;
}

V2vClusterSap::DMACJoin
V2vModifiedDMACAlgorithmClient::CreateJoinPacket(void){

    V2vClusterSap::DMACJoin joinStruct;
    joinStruct.ttl = m_ttl;
    joinStruct.id = m_currentInfo.id;
    joinStruct.CHindex = m_currentInfo.CHindex;

    return joinStruct;
}

void
V2vModifiedDMACAlgorithmClient::CreateForwardedHelloPacket(V2vClusterSap::DMACHello oldMessage){

    m_forwardHello.id = oldMessage.id;
    m_forwardHello.weight = oldMessage.weight;
    m_forwardHello.position = oldMessage.position;
    m_forwardHello.velocity = oldMessage.velocity;
    m_forwardHello.direction = oldMessage.direction;
    m_forwardHello.role = oldMessage.role;
    m_forwardHello.ttl = oldMessage.ttl-1;
}

void
V2vModifiedDMACAlgorithmClient::CreateForwardedCHPacket(V2vClusterSap::DMACCH oldMessage){

    m_forwardCH.id = m_currentInfo.id;
    m_forwardCH.ttl = oldMessage.ttl - 1;
}

void
V2vModifiedDMACAlgorithmClient::CreateForwardedJoinPacket(V2vClusterSap::DMACJoin oldMessage){

    m_forwardJoin.id = oldMessage.id;
    m_forwardJoin.ttl = oldMessage.ttl - 1;
}

V2vClusterSap::DMACNeighbours
V2vModifiedDMACAlgorithmClient::CreateNeighbor(Time ts, V2vClusterSap::DMACHello helloMessage){
    V2vClusterSap::DMACNeighbours neighbor;

    neighbor.weight = helloMessage.weight;
    neighbor.position = helloMessage.position;
    neighbor.velocity = helloMessage.velocity;
    neighbor.direction = helloMessage.direction;
    neighbor.role = helloMessage.role;
    neighbor.tExpire = ts;
    neighbor.beats = CalculateFreshness(helloMessage.position, helloMessage.velocity);

    return neighbor;
}

V2vClusterSap::DMACNeighbours
V2vModifiedDMACAlgorithmClient::CreateClusterNode(Time ts, V2vClusterSap::DMACNeighbours neighbor){
    V2vClusterSap::DMACNeighbours member;

    member.tExpire = ts;
    member.weight = neighbor.weight;
    member.position = neighbor.position;
    member.velocity = neighbor.velocity;
    member.direction = neighbor.direction;
    member.role = neighbor.role;
    member.beats = CalculateFreshness(neighbor.position, neighbor.velocity);

    return member;
}

bool
V2vModifiedDMACAlgorithmClient::TestClusterHeadChange(uint64_t id, V2vClusterSap::DMACNeighbours neighbor){

    if(m_currentInfo.CHindex == id){
        return false;
    }

    uint64_t weight = 0;
    std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator found = m_neighborMap.find (m_currentInfo.CHindex);
    if(found == m_neighborMap.end ()){
        if(m_currentInfo.role == V2vClusterSap::CH){
            weight = m_currentInfo.weight;
        }
    }
    else{
        weight = found->second.weight;
    }
    if(neighbor.weight > weight){
        NS_LOG_DEBUG("Weight:" << neighbor.weight);
        return false;
    }

    if(neighbor.role == V2vClusterSap::CM){
        NS_LOG_DEBUG("Role:" << ToString (neighbor.role));
        return false;
    }

    if(neighbor.beats > m_freshnessThreshold){
        NS_LOG_DEBUG("Beats:" << neighbor.beats);
        return false;
    }

    if(IsSameDirection (neighbor.velocity)){
        NS_LOG_DEBUG("Direction:" << neighbor.direction);
        return false;
    }

    return true;
}

double
V2vModifiedDMACAlgorithmClient::CalculateWeight(void){

    double size = m_neighborMap.size();

    //!< If no neighbours found, return standard TDMA vehicle's window
    if(m_neighborMap.size() == 0){
        size = 1;
    }

    Vector p,v;
    for(std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it) {
        uint64_t key = it->first;
        NS_ASSERT(key != m_currentInfo.id);

        V2vClusterSap::DMACNeighbours value = it->second;
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
    for(std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it) {
        uint64_t key = it->first;
        NS_ASSERT(key != m_currentInfo.id);

        V2vClusterSap::DMACNeighbours value = it->second;
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
    NS_LOG_DEBUG("w = " << w);

    return w;
}

bool
V2vModifiedDMACAlgorithmClient::IsSameDirection(Vector otherVelocity){

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

template <typename T> int V2vModifiedDMACAlgorithmClient::sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

double
V2vModifiedDMACAlgorithmClient::CalculateFreshness (Vector otherPosition, Vector otherVelocity){

    double freshness = m_freshness;

    double l = CalculateDistance (otherPosition, m_currentInfo.position);
    double u = CalculateDistance (otherVelocity, m_currentInfo.velocity);
    double a = u*u;
    if(a > 0){
        double b = 2*u*l;
        double c = (l*l) - (m_range*m_range);
        double dt = SolveQuadratic(a, b, c);
        freshness = std::min(3*freshness, dt);
        NS_LOG_DEBUG("Freshness is: " << freshness);
    }

    return freshness;
}

double
V2vModifiedDMACAlgorithmClient::SolveQuadratic(double a, double b, double c){
    double sqrtpart = b*b - 4*a*c;
    double x1 = 0.0;
    if(sqrtpart > 0){
        x1 = (-b + sqrt(sqrtpart)) / (2 * a);
    }

    return x1;
}

void
V2vModifiedDMACAlgorithmClient::ConnectionSucceeded (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vModifiedDMACAlgorithmClient::ChangeState(V2vClusterSap::DMACNodeState nextState){
    m_nodeState = nextState;
    NS_LOG_DEBUG("Node: " << m_currentInfo.id << " enters in state: " << ToString (m_nodeState) << " at Time: " << Simulator::Now ().GetSeconds ());
}

void
V2vModifiedDMACAlgorithmClient::StatusReport (void){
    NS_LOG_UNCOND("\n\n-----------------------------------------------------------------------------");
    NS_LOG_UNCOND ("[StatusReport] => At time " << Simulator::Now ().GetSeconds ()
                   << "s node ["<< m_currentInfo.id << "] is: " << ToString (m_currentInfo.role) << " in cluster:" << m_currentInfo.CHindex
                   << " has  ===> \n position: " << m_currentInfo.position
                   << " - Velocity: " << m_currentInfo.velocity << " - Direction: " << m_currentInfo.direction
                   << "\n");

    NS_LOG_UNCOND("----------------------------  Neighbors Info  ---------------------------------");
    NS_LOG_UNCOND("NeighborMap size:" << m_neighborMap.size ());
    for(std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it){
        uint64_t id = it->first;
        V2vClusterSap::DMACNeighbours node = it->second;
        NS_LOG_UNCOND(" * key: " << id
                << "tExpire:" << node.tExpire.GetSeconds()
                << " Role:" << ToString (node.role)
                << " Weight:" << node.weight
                << " Position:" << node.position
                << " Velocity:" << node.velocity
                << " Direction:" << node.direction);
    }

    NS_LOG_UNCOND("----------------------------  Cluster Info  ---------------------------------");
    for(std::map<uint64_t, V2vClusterSap::DMACNeighbours>::iterator it = m_clusterMap.begin(); it != m_clusterMap.end(); ++it){
        uint64_t id = it->first;
        V2vClusterSap::DMACNeighbours node = it->second;
        NS_LOG_UNCOND(" * key: " << id
                << " Role:" << ToString (node.role)
                << " Weight:" << node.weight
                << " Position:" << node.position
                << " Velocity:" << node.velocity
                << " Direction:" << node.direction);
    }
}


} // Namespace ns3
