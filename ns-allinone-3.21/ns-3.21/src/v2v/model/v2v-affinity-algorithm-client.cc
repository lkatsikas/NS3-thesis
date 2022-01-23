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
#include "v2v-affinity-algorithm-client.h"

#include "ns3/random-variable.h"


namespace ns3 {

static const std::string
StateName[V2vClusterSap::AFFINITY_STATES] =
{
    "UNDEFINED",
    "TH",
    "TM",
    "TCM"
};

static const std::string & ToString (V2vClusterSap::AffinityNodeState state){
    return StateName[state];
}

NS_LOG_COMPONENT_DEFINE ("V2vAffinityAlgorithmClient");
NS_OBJECT_ENSURE_REGISTERED (V2vAffinityAlgorithmClient);

TypeId V2vAffinityAlgorithmClient::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vAffinityAlgorithmClient").SetParent<Application>()
            .AddConstructor<V2vAffinityAlgorithmClient>()
            .AddAttribute("ListeningLocal",
                    "The Address on which to Bind the rx socket.",
                    AddressValue(), MakeAddressAccessor(&V2vAffinityAlgorithmClient::m_peerListening),
                    MakeAddressChecker())
            .AddAttribute("ProtocolListeningLocal",
                    "The type id of the protocol to use for the rx socket.",
                    TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vAffinityAlgorithmClient::m_tidListening),
                    MakeTypeIdChecker())
            .AddTraceSource("RxLocal", "A packet has been received",
                    MakeTraceSourceAccessor(&V2vAffinityAlgorithmClient::m_rxTrace))

            .AddAttribute("TrainingPeriod",
                    "Training period of time", DoubleValue(80.0),
                    MakeDoubleAccessor(&V2vAffinityAlgorithmClient::m_trainingPeriod),
                    MakeDoubleChecker<double>())
            .AddAttribute("Tf",
                    "Position calculation time window", DoubleValue(1.0),
                    MakeDoubleAccessor(&V2vAffinityAlgorithmClient::m_tf),
                    MakeDoubleChecker<double>())
            .AddAttribute("Lamda",
                    "Lamba dumping factor", DoubleValue(0.5),
                    MakeDoubleAccessor(&V2vAffinityAlgorithmClient::m_lamda),
                    MakeDoubleChecker<double>())
            .AddAttribute("VehicleTdmaSlot",
                    "The vehicle's' TDMA window", DoubleValue(0.1),
                    MakeDoubleAccessor(&V2vAffinityAlgorithmClient::m_vehicleTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("CI",
                    "Cluster Interval period time", UintegerValue(10),
                    MakeUintegerAccessor(&V2vAffinityAlgorithmClient::m_CIperiod),
                    MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("MinimumTdmaSlot",
                    "The maximun size of the TDMA window", DoubleValue(0.001),
                    MakeDoubleAccessor(&V2vAffinityAlgorithmClient::m_minimumTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("SelfSimilarity",
                    "Priority factor to become exemplar", DoubleValue(-30.0),
                    MakeDoubleAccessor(&V2vAffinityAlgorithmClient::m_selfSimilarity),
                    MakeDoubleChecker<double>())
            .AddAttribute("MaxUes",
                    "The maximun size of hehicles permitted", UintegerValue(100),
                    MakeUintegerAccessor(&V2vAffinityAlgorithmClient::m_maxUes),
                    MakeUintegerChecker<uint32_t>(1))
            .AddAttribute ("Interval",
                    "The time to wait between packets", TimeValue (Seconds (1.0)),
                    MakeTimeAccessor (&V2vAffinityAlgorithmClient::m_interval),
                    MakeTimeChecker ())
            .AddAttribute("SendingLocal",
                    "The address of the destination", AddressValue(),
                    MakeAddressAccessor(&V2vAffinityAlgorithmClient::m_peer),
                    MakeAddressChecker())
            .AddAttribute("ProtocolSendingLocal",
                    "The type of protocol for the tx socket.",
                    TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vAffinityAlgorithmClient::m_tid),
                    MakeTypeIdChecker())
            .AddAttribute ("MobilityModel",
                    "The mobility model of the node.",
                    PointerValue (),
                    MakePointerAccessor (&V2vAffinityAlgorithmClient::m_mobilityModel),
                    MakePointerChecker<MobilityModel> ())
            .AddTraceSource("TxLocal","A new packet is created and is sent",
                    MakeTraceSourceAccessor(&V2vAffinityAlgorithmClient::m_txTrace));
    return tid;
}


// Public Members
V2vAffinityAlgorithmClient::V2vAffinityAlgorithmClient () {
    NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    m_clusterChanges = 0;
    m_numberOfMessages = 0;
    m_clusterChangesPerSec = 0;

    m_sentCounter = 0;
    m_receivedCounter = 0;

    m_tf = 0.0;
    m_neighborMap.clear ();
    m_sendEvent = EventId ();
}

V2vAffinityAlgorithmClient::~V2vAffinityAlgorithmClient () {
    NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    m_sentCounter = 0;
    m_receivedCounter = 0;

    m_tf = 0.0;
    m_neighborMap.clear ();
    m_sendEvent.Cancel ();
}

double
V2vAffinityAlgorithmClient::GetClusterMembers(void){

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
V2vAffinityAlgorithmClient::GetFormationDelay (){

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
V2vAffinityAlgorithmClient::GetClusterChanges (void){
    return m_clusterChanges;
}

uint64_t
V2vAffinityAlgorithmClient::GetNumberOfMessages (void){
    return m_numberOfMessages;
}

bool
V2vAffinityAlgorithmClient::GetRole (void){
    if(m_currentInfo.CHindex == m_currentInfo.id){
        return true;
    }
    return false;
}

uint64_t
V2vAffinityAlgorithmClient::GetNumberOfMessagesPerSecond (void){
    uint64_t tmp = m_numberOfMessagesPerSec;
    m_numberOfMessagesPerSec = 0;

    return tmp;
}

uint64_t V2vAffinityAlgorithmClient::GetClusterChangesPerSecond (void){
    uint64_t tmp = m_clusterChangesPerSec;
    m_clusterChangesPerSec = 0;

    return tmp;
}

double
V2vAffinityAlgorithmClient::GetChDuration (void){

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
V2vAffinityAlgorithmClient::GetCmDuration (void){

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
V2vAffinityAlgorithmClient::GetMinChDuration (void){

    uint64_t min = Simulator::Now ().GetTimeStep ();
    for(std::vector<uint64_t>::iterator it = m_chDurationVector.begin (); it != m_chDurationVector.end (); ++it){
        if((*it) < min){
            min = *it;
        }
    }
    return TimeStep (min).GetSeconds ();
}

double
V2vAffinityAlgorithmClient::GetMaxChDuration (void){

    uint64_t max = 0;
    for(std::vector<uint64_t>::iterator it = m_chDurationVector.begin (); it != m_chDurationVector.end (); ++it){
        if((*it) > max){
            max = *it;
        }
    }
    return TimeStep (max).GetSeconds ();
}

double
V2vAffinityAlgorithmClient::GetMinCmDuration (void){

    uint64_t min = Simulator::Now ().GetTimeStep ();
    for(std::vector<uint64_t>::iterator it = m_cmDurationVector.begin (); it != m_cmDurationVector.end (); ++it){
        if((*it) < min){
            min = *it;
        }
    }
    return TimeStep (min).GetSeconds ();
}

double
V2vAffinityAlgorithmClient::GetMaxCmDuration (void){

    uint64_t max = 0;
    for(std::vector<uint64_t>::iterator it = m_cmDurationVector.begin (); it != m_cmDurationVector.end (); ++it){
        if((*it) > max){
            max = *it;
        }
    }
    return TimeStep (max).GetSeconds ();
}

void
V2vAffinityAlgorithmClient::PrintStatistics (std::ostream &os){

    os << "********************************************************" << std::endl
       << "  - Cluster Metrics - for"  << " Node:" << m_currentInfo.id  << std::endl
       << " Overall Sent messages: " << m_sentCounter << std::endl
       << " Overall Received messages: " << m_receivedCounter << std::endl
       << "********************************************************" << std::endl;

    os << "\n********************************************************" << std::endl
       << "  - Average Cluster Formation Delay:" << GetFormationDelay () <<  std::endl
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
V2vAffinityAlgorithmClient::DoDispose (void) {
    NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    // chain up
    Application::DoDispose();
}

void
V2vAffinityAlgorithmClient::StartApplication (void)
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
                MakeCallback(&V2vAffinityAlgorithmClient::ConnectionSucceeded, this),
                MakeCallback(&V2vAffinityAlgorithmClient::ConnectionFailed, this));
    }

    if(m_maxUes > 100){
        NS_FATAL_ERROR("Error: Maximum number of ues is 100.");
    }

    StartListeningLocal();

    ChangeState (m_nodeState);
    Simulator::Schedule(Seconds(m_trainingPeriod), &V2vAffinityAlgorithmClient::AcquireMobilityInfo, this);
    ScheduleTransmit (Seconds (m_trainingPeriod + m_vehicleTdmaSlot));

}

void
V2vAffinityAlgorithmClient::StartListeningLocal (void)    // Called at time specified by Start
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

    m_socketListening->SetRecvCallback(MakeCallback(&V2vAffinityAlgorithmClient::HandleRead, this));
    m_socketListening->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback(&V2vAffinityAlgorithmClient::HandleAccept, this));
    m_socketListening->SetCloseCallbacks(
            MakeCallback(&V2vAffinityAlgorithmClient::HandlePeerClose, this),
            MakeCallback(&V2vAffinityAlgorithmClient::HandlePeerError, this));
}

void
V2vAffinityAlgorithmClient::StopApplication (void) // Called at time specified by Stop
{
    NS_LOG_FUNCTION (this);

    if (m_socket != 0) {
        m_socket->Close();
        m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
        m_socket = 0;
    } else {
        NS_LOG_WARN ("V2vAffinityAlgorithmClient found null socket to close in StopApplication");
    }
    Simulator::Cancel (m_sendEvent);
    StopListeningLocal();
    PrintStatistics(std::cout);
    StatusReport ();
}

void
V2vAffinityAlgorithmClient::StopListeningLocal (void)     // Called at time specified by Stop
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
V2vAffinityAlgorithmClient::GetListeningSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socketListening;
}

Ptr<Socket>
V2vAffinityAlgorithmClient::GetSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socket;
}


// Private Members
void
V2vAffinityAlgorithmClient::HandleRead (Ptr<Socket> socket) {
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

            if(item.tid.GetName() == "ns3::V2vAffinityHelloHeader"){
                V2vAffinityHelloHeader helloHeader;
                p->RemoveHeader (helloHeader);

                NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Received Hello message from: " << helloHeader.GetHelloInfo ().id);
                if(IsSameDirection (helloHeader.GetHelloInfo ().velocity)){
                    m_neighborMap[helloHeader.GetHelloInfo().id] = CreateNeighbor(helloHeader.GetTs (), helloHeader.GetHelloInfo ());
                }
            }
            else if(item.tid.GetName() == "ns3::V2vAffinityRespAvailHeader"){
                V2vAffinityRespAvailHeader respAvailHeader;
                p->RemoveHeader (respAvailHeader);

                NS_LOG_DEBUG ("Node:" << m_currentInfo.id << " From:" << respAvailHeader.GetRespAvailInfo ().id << " Received RespAvail at time: " << respAvailHeader.GetTs() << " respMap size is: " << respAvailHeader.GetRespAvailList().size());
                std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator found = m_neighborMap.find (respAvailHeader.GetRespAvailInfo ().id);
                if(found != m_neighborMap.end ()){
                    std::list<V2vClusterSap::AffinitySubStructure> listCopy = respAvailHeader.GetRespAvailList();
                    for (std::list<V2vClusterSap::AffinitySubStructure>::iterator tmp = listCopy.begin(); tmp != listCopy.end(); ++tmp) {
                        NS_LOG_DEBUG("tmp->id:" << tmp->id << " - tmp->resp:" << tmp->resp << " - tmp->avail:" << tmp->avail);
                        if(tmp->id == m_currentInfo.id){
                            found->second.responsibilityReceived = tmp->resp;
                            found->second.availabilityReceived = tmp->avail;
                        }
                    }
                    found->second.CHcnvg = respAvailHeader.GetRespAvailInfo ().CHcnvg;
                }
            }
            m_rxTrace(p, from);
        }
        m_receivedCounter ++;
    }
}

void
V2vAffinityAlgorithmClient::HandleAccept (Ptr<Socket> s, const Address& from) {
    NS_LOG_FUNCTION (this << s << from);
    s->SetRecvCallback(MakeCallback(&V2vAffinityAlgorithmClient::HandleRead, this));
}

void
V2vAffinityAlgorithmClient::HandlePeerClose (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vAffinityAlgorithmClient::HandlePeerError (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vAffinityAlgorithmClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &V2vAffinityAlgorithmClient::Send, this);
}

void
V2vAffinityAlgorithmClient::Send (void) {
    NS_LOG_FUNCTION (this);
    NS_ASSERT(m_sendEvent.IsExpired());

    switch (m_nodeState) {
    case V2vClusterSap::TH:{

        V2vAffinityHelloHeader helloHeader;
        helloHeader.SetHelloInfo(CreateHelloPacket());

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader (helloHeader);
        m_socket->Send (packet);
        m_txTrace(packet);
        m_sentCounter ++;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends Hello Message at : " << helloHeader.GetTs ().GetSeconds ());
        ChangeState (m_nodeState);
        ScheduleTransmit (Seconds(m_maxUes*m_minimumTdmaSlot));

        break;
    }
    case V2vClusterSap::TM:{

        if(m_formationDelayBoolean == false){
            m_startFormationDelay = Simulator::Now ().GetTimeStep ();
            m_formationDelayBoolean = true;
        }

        V2vAffinityRespAvailHeader respAvailHeader;
        respAvailHeader.SetRespAvailList (CreateRespAvailList());
        respAvailHeader.SetRespAvailInfo (CreateRespAvailInfo());

        Ptr<Packet> packet = Create<Packet>(0);
        packet->AddHeader (respAvailHeader);
        m_socket->Send (packet);
        m_txTrace(packet);
        m_sentCounter ++;

        m_numberOfMessages ++;
        m_numberOfMessagesPerSec ++;

        NS_LOG_DEBUG("Node:" << m_currentInfo.id << " Sends TM Message at : " << respAvailHeader.GetTs ().GetSeconds ());

        ChangeState (m_nodeState);
        ScheduleTransmit (Seconds(m_maxUes*m_minimumTdmaSlot));

        break;
    }
    case V2vClusterSap::TCM:{

        PurgeNeighbours();

        if(((uint32_t)Simulator::Now ().GetSeconds () % (uint32_t)(m_CIperiod) == 0) && ((uint32_t)Simulator::Now ().GetSeconds () != 0)){
            NS_LOG_DEBUG ("Start CI ==========================================>");
            CIMaintenanceTasks();
        }
        else{
            NS_LOG_DEBUG ("Start TCM ==========================================>");
            PeriodicMaintenanceTasks();
        }

        //StatusReport ();
        AcquireMobilityInfo();
        double elapsed = Simulator::Now ().GetSeconds () - (int)Simulator::Now ().GetSeconds ();

        ChangeState (m_nodeState);
        ScheduleTransmit (Seconds(m_interval.GetSeconds () - elapsed + m_vehicleTdmaSlot));
        break;
    }
    default:
        break;
    }
}

void
V2vAffinityAlgorithmClient::ConnectionFailed (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vAffinityAlgorithmClient::AcquireMobilityInfo (void){

    NS_LOG_DEBUG("Update Mobility Info");

    //!< Acquire current mobility stats
    m_currentInfo.selfSim = m_selfSimilarity;
    m_currentInfo.id = this->GetNode ()->GetId ();
    m_currentInfo.position = m_mobilityModel->GetPosition();
    m_currentInfo.velocity = m_mobilityModel->GetVelocity();
    m_currentInfo.direction = Vector(0.0, 0.0, 0.0);//m_currentInfo.direction = m_mobilityModel->GetDirection();
}

V2vClusterSap::AffinityHello
V2vAffinityAlgorithmClient::CreateHelloPacket(void){
    V2vClusterSap::AffinityHello helloStruct;
    helloStruct.id = m_currentInfo.id;
    helloStruct.CHindex = m_currentInfo.CHindex;
    helloStruct.position = m_currentInfo.position;
    helloStruct.velocity = m_currentInfo.velocity;
    helloStruct.direction = m_currentInfo.direction;

    return helloStruct;
}

V2vClusterSap::AffinityRespAvail
V2vAffinityAlgorithmClient::CreateRespAvailInfo(void){

    V2vClusterSap::AffinityRespAvail respAvailStruct;
    respAvailStruct.id = m_currentInfo.id;
    respAvailStruct.CHcnvg = IsConverged();
    respAvailStruct.neighboursNumber = m_neighborMap.size ();

    return respAvailStruct;
}

std::list<V2vClusterSap::AffinitySubStructure>
V2vAffinityAlgorithmClient::CreateRespAvailList(void){

    // find self responsibility - self availability
    double sumAvail = 0.0;
    double maxResp = -1e100;
    for(std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it) {
        V2vClusterSap::AffinityNeighbors value = it->second;
        if((value.similarity + value.availabilityReceived) > maxResp){
            maxResp = value.similarity + value.availabilityReceived;
        }
        sumAvail += std::max (0.0, value.responsibilityReceived);
    }
    m_currentInfo.selfResp = ((1-m_lamda)*(m_selfSimilarity - maxResp)) + (m_lamda*m_currentInfo.selfResp);
    m_currentInfo.selfAvail = ((1-m_lamda)*(sumAvail)) + (m_lamda*m_currentInfo.selfAvail);
    NS_LOG_DEBUG("Self responsibility is:" << m_currentInfo.selfResp << " - Self availaility is:" << m_currentInfo.selfAvail);


    std::list<V2vClusterSap::AffinitySubStructure> respAvailList;
    std::map<uint64_t, V2vClusterSap::AffinityNeighbors> neighbourMap2 = m_neighborMap;
    for(std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it) {

        V2vClusterSap::AffinityNeighbors value = it->second;

        // find each neighbour responsibility - availability
        sumAvail = 0.0;
        maxResp = -1e100;
        for(std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator it2 = neighbourMap2.begin(); it2 != neighbourMap2.end(); ++it2) {
            V2vClusterSap::AffinityNeighbors value2 = it2->second;
            if(it->first != it2->first){
                if((value2.similarity + value2.availabilityReceived) > maxResp){
                    maxResp = value2.similarity + value2.availabilityReceived;
                }
                sumAvail += std::max (0.0, value2.responsibilityReceived);
            }
        }
        if((value.similarity + value.availabilityReceived) > maxResp){
            maxResp = value.similarity + value.availabilityReceived;
        }

        it->second.responsibilitySent = ((1-m_lamda)*(value.similarity - maxResp)) + (m_lamda*value.responsibilitySent);
        NS_LOG_DEBUG("it->second.responsibilitySent for neighbor:" << it->first << " is: " << it->second.responsibilitySent);

        it->second.availabilitySent = (1-m_lamda)*std::min (0.0, value.responsibilityReceived + sumAvail) + (m_lamda*value.availabilitySent);
        NS_LOG_DEBUG("it->second.it->second.availabilitySent for neighbor:" << it->first << " is: " << it->second.availabilitySent);

        V2vClusterSap::AffinitySubStructure tmp;
        tmp.id = it->first;
        tmp.resp = it->second.responsibilitySent;
        tmp.avail = it->second.availabilitySent;
        respAvailList.push_back (tmp);
    }

    return respAvailList;
}

V2vClusterSap::AffinityNeighbors
V2vAffinityAlgorithmClient::CreateNeighbor(Time ts, V2vClusterSap::AffinityHello helloMessage){
    V2vClusterSap::AffinityNeighbors neighbor;
    neighbor.tExpire = ts;
    neighbor.CHindex = helloMessage.CHindex;
    neighbor.position = helloMessage.position;
    neighbor.velocity = helloMessage.velocity;
    neighbor.direction = helloMessage.direction;
    neighbor.similarity = CalculateSimilarity(helloMessage.position, helloMessage.velocity);

    return neighbor;
}

bool
V2vAffinityAlgorithmClient::IsSameDirection(Vector otherVelocity){

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

template <typename T> int V2vAffinityAlgorithmClient::sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

double
V2vAffinityAlgorithmClient::CalculateSimilarity(Vector otherPosition, Vector otherVelocity){

    Vector myNextPos(0.0, 0.0, 0.0);
    myNextPos.x = m_currentInfo.position.x + m_currentInfo.velocity.x*m_tf;
    myNextPos.y = m_currentInfo.position.y + m_currentInfo.velocity.y*m_tf;
    myNextPos.z = m_currentInfo.position.z + m_currentInfo.velocity.z*m_tf;

    Vector otherNextPos(0.0, 0.0, 0.0);
    otherNextPos.x = otherPosition.x + otherVelocity.x*m_tf;
    otherNextPos.y = otherPosition.y + otherVelocity.y*m_tf;
    otherNextPos.z = otherPosition.z + otherVelocity.z*m_tf;

    double similarity = -(CalculateDistance (m_currentInfo.position, otherPosition) + CalculateDistance (myNextPos, otherNextPos));
    NS_LOG_DEBUG("Similarity function is:" << similarity);

    return similarity;
}

bool
V2vAffinityAlgorithmClient::IsConverged(void){
    if( (m_currentInfo.selfResp + m_currentInfo.selfAvail) > 0){
        m_currentInfo.CHcnvg = true;

        m_stopFormationDelay = Simulator::Now ().GetTimeStep ();
        m_formationDelayBoolean = false;
        m_formationDelayVector.push_back (m_stopFormationDelay - m_startFormationDelay);
    }
    else{
        m_currentInfo.CHcnvg = false;
    }

    return m_currentInfo.CHcnvg;
}

void
V2vAffinityAlgorithmClient::CIMaintenanceTasks(void){

    uint64_t previousIndex = m_currentInfo.CHindex;

    if(m_currentInfo.CHcnvg){
        m_currentInfo.CHindex = m_currentInfo.id;
    }
    else{
        double maxSim = -1e100;
        uint64_t bestNeighbour = 0;
        for(std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it){
            V2vClusterSap::AffinityNeighbors node = it->second;
            if(node.CHcnvg){
                if(node.responsibilitySent+node.availabilityReceived > maxSim){
                    maxSim = node.responsibilitySent+node.availabilityReceived;
                    bestNeighbour = it->first;
                }
            }
        }

        // if not neighbours available for CH, turn on myself
        if(bestNeighbour == 0){
            m_currentInfo.CHindex = m_currentInfo.id;
        }
        else{
            m_currentInfo.CHindex = bestNeighbour;
        }
    }

    if(previousIndex != m_currentInfo.CHindex){
        m_clusterChanges ++;
        m_clusterChangesPerSec ++;

        if(m_currentInfo.id != m_currentInfo.CHindex){
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
}

void
V2vAffinityAlgorithmClient::PeriodicMaintenanceTasks(void){

    uint64_t previousIndex = m_currentInfo.CHindex;

    if(m_currentInfo.CHindex == m_currentInfo.id){
        uint64_t sum = 0;
        for(std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it){
            V2vClusterSap::AffinityNeighbors value = it->second;
            if(value.CHindex == m_currentInfo.id){
                sum ++;
            }
        }
        m_clusterMembers.push_back (sum+1);
    }

    if(m_currentInfo.CHindex != 0){

        if(m_currentInfo.CHindex != m_currentInfo.id){
            std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator found = m_neighborMap.find (m_currentInfo.CHindex);
            if(found == m_neighborMap.end ()){

                double maxSim = -1e100;
                uint64_t bestNeighbour = 0;
                for(std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it){
                    V2vClusterSap::AffinityNeighbors node = it->second;
                    if(node.responsibilitySent+node.availabilityReceived > maxSim){
                        maxSim = node.responsibilitySent+node.availabilityReceived;
                        bestNeighbour = it->first;
                    }
                }

                if(bestNeighbour == 0){
                    m_currentInfo.CHindex = m_currentInfo.id;
                }
                else{
                    m_currentInfo.CHindex = bestNeighbour;
                }
            }
        }
    }

    if(previousIndex != m_currentInfo.CHindex){
        m_clusterChanges ++;
        m_clusterChangesPerSec ++;

        if(m_currentInfo.id != m_currentInfo.CHindex){
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
}

void
V2vAffinityAlgorithmClient::PurgeNeighbours (void) {

    for(std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end();){

        V2vClusterSap::AffinityNeighbors node = it->second;
        if(Simulator::Now ().GetSeconds () - node.tExpire.GetSeconds () > 1.5){
            m_neighborMap.erase (it++);
        }
        else{
            ++it;
        }
    }
}

void
V2vAffinityAlgorithmClient::ConnectionSucceeded (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vAffinityAlgorithmClient::ChangeState(V2vClusterSap::AffinityNodeState currentState){
    switch (currentState) {
    case V2vClusterSap::TH:
        m_nodeState = V2vClusterSap::TM;
        break;
    case V2vClusterSap::TM:
        m_nodeState = V2vClusterSap::TCM;
        break;
    case V2vClusterSap::TCM:
        m_nodeState = V2vClusterSap::TH;
        break;
    default:
        m_nodeState = V2vClusterSap::TH;
        break;
    }
    NS_LOG_DEBUG("Node: " << m_currentInfo.id << " enters in state: " << ToString (m_nodeState) << " at Time: " << Simulator::Now ().GetSeconds ());
}

void
V2vAffinityAlgorithmClient::StatusReport (void){
    NS_LOG_UNCOND("\n\n-----------------------------------------------------------------------------");
    NS_LOG_UNCOND ("[StatusReport] => At time " << Simulator::Now ().GetSeconds ()
                   << "s node ["<< m_currentInfo.id << "] is in cluster:" << m_currentInfo.CHindex
                   << " has  ===> \n position: " << m_currentInfo.position
                   << " - Velocity: " << m_currentInfo.velocity << " - Direction: " << m_currentInfo.direction
                   << " SelfSimilarity is: " << m_currentInfo.selfSim
                   << " SelfResponsibility is: " << m_currentInfo.selfResp
                   << " SelfAvailability is: " << m_currentInfo.selfAvail << "\n");

    NS_LOG_UNCOND("----------------------------  Neighbors Info  ---------------------------------");
    NS_LOG_UNCOND("NeighborMap size:" << m_neighborMap.size ());
    for(std::map<uint64_t, V2vClusterSap::AffinityNeighbors>::iterator it = m_neighborMap.begin(); it != m_neighborMap.end(); ++it){
        uint64_t id = it->first;
        V2vClusterSap::AffinityNeighbors node = it->second;
        NS_LOG_UNCOND(" * key: " << id
                << " tExpire:" << node.tExpire.GetSeconds ()
                << " CHcvng:" << node.CHcnvg
                << " CHindex:" << node.CHindex
                << " Position:" << node.position
                << " Velocity:" << node.velocity
                << " Direction:" << node.direction
                << " Similarity:" << node.similarity
                << " AvailSent:" << node.availabilitySent
                << " AvailReceived:" << node.availabilityReceived
                << " RespSent:" << node.responsibilitySent
                << " RespReceived:" << node.responsibilityReceived);
    }
}


} // Namespace ns3
