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
#include "v2v-general-client.h"

#include "ns3/random-variable.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("V2vGeneralClient");
NS_OBJECT_ENSURE_REGISTERED (V2vGeneralClient);

TypeId V2vGeneralClient::GetTypeId(void) {
	static TypeId tid =
            TypeId("ns3::V2vGeneralClient").SetParent<Application>()
            .AddConstructor<V2vGeneralClient>()
            .AddAttribute("ListeningLocal",
					"The Address on which to Bind the rx socket.",
                    AddressValue(), MakeAddressAccessor(&V2vGeneralClient::m_peerListening),
					MakeAddressChecker())
            .AddAttribute("ProtocolListeningLocal",
					"The type id of the protocol to use for the rx socket.",
					TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vGeneralClient::m_tidListening),
					MakeTypeIdChecker())
            .AddTraceSource("RxLocal", "A packet has been received",
                    MakeTraceSourceAccessor(&V2vGeneralClient::m_rxTrace))

            .AddAttribute("VehicleTdmaSlot",
                    "The vehicle's' TDMA window", DoubleValue(0.1),
                    MakeDoubleAccessor(&V2vGeneralClient::m_vehicleTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("MinimumTdmaSlot",
                    "The maximun size of the TDMA window", DoubleValue(0.001),
                    MakeDoubleAccessor(&V2vGeneralClient::m_minimumTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("MaxUes",
                    "The maximun size of ues permitted", UintegerValue(100),
                    MakeUintegerAccessor(&V2vGeneralClient::m_maxUes),
                    MakeUintegerChecker<uint32_t>(1))
            .AddAttribute ("Interval",
                    "The time to wait between packets", TimeValue (Seconds (1.0)),
                    MakeTimeAccessor (&V2vGeneralClient::m_interval),
                    MakeTimeChecker ())
            .AddAttribute("SendingLocal",
					"The address of the destination", AddressValue(),
                    MakeAddressAccessor(&V2vGeneralClient::m_peer),
					MakeAddressChecker())
            .AddAttribute("ProtocolSendingLocal",
					"The type of protocol for the tx socket.",
					TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vGeneralClient::m_tid),
					MakeTypeIdChecker())
			.AddAttribute ("MobilityModel",
				    "The mobility model of the node.",
				    PointerValue (),
                    MakePointerAccessor (&V2vGeneralClient::m_mobilityModel),
                    MakePointerChecker<V2vMobilityModel> ())
            .AddTraceSource("TxLocal","A new packet is created and is sent",
                    MakeTraceSourceAccessor(&V2vGeneralClient::m_txTrace));
	return tid;
}


// Public Members
V2vGeneralClient::V2vGeneralClient () {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    m_sentCounter = 0;
    m_receivedCounter = 0;
    m_sendEvent = EventId ();
}

V2vGeneralClient::~V2vGeneralClient () {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

    m_sentCounter = 0;
    m_receivedCounter = 0;
}

void
V2vGeneralClient::PrintStatistics (std::ostream &os){

    os << "***********************" << std::endl
       << "  - Cluster Metrics -  " << std::endl
       << "Node:" << m_currentMobility.id << " Sent overal: " << m_sentCounter << " Packets." << std::endl
       << "***********************" << std::endl;
}

// Protected Members
void
V2vGeneralClient::DoDispose (void) {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

	// chain up
	Application::DoDispose();
}

void
V2vGeneralClient::StartApplication (void)
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
                MakeCallback(&V2vGeneralClient::ConnectionSucceeded, this),
                MakeCallback(&V2vGeneralClient::ConnectionFailed, this));
    }

    if(m_maxUes > 100){
        NS_FATAL_ERROR("Error: Maximum number of ues is 100.");
    }

    StartListeningLocal();
    AcquireMobilityInfo();

    ScheduleTransmit (Seconds (m_vehicleTdmaSlot));
}

void
V2vGeneralClient::StartListeningLocal (void)    // Called at time specified by Start
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

    m_socketListening->SetRecvCallback(MakeCallback(&V2vGeneralClient::HandleRead, this));
	m_socketListening->SetAcceptCallback(
			MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback(&V2vGeneralClient::HandleAccept, this));
	m_socketListening->SetCloseCallbacks(
            MakeCallback(&V2vGeneralClient::HandlePeerClose, this),
            MakeCallback(&V2vGeneralClient::HandlePeerError, this));
}

void
V2vGeneralClient::StopApplication (void) // Called at time specified by Stop
{
	NS_LOG_FUNCTION (this);

	if (m_socket != 0) {
		m_socket->Close();
		m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
		m_socket = 0;
	} else {
        NS_LOG_WARN ("V2vGeneralClient found null socket to close in StopApplication");
	}
	Simulator::Cancel (m_sendEvent);
    StopListeningLocal();
    PrintStatistics(std::cout);
}

void
V2vGeneralClient::StopListeningLocal (void)     // Called at time specified by Stop
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
V2vGeneralClient::GetListeningSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socketListening;
}

Ptr<Socket>
V2vGeneralClient::GetSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socket;
}


// Private Members
void
V2vGeneralClient::HandleRead (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
	Ptr<Packet> packet;
	Address from;
	while ((packet = socket->RecvFrom(from))) {
		if (packet->GetSize() == 0) { //EOF
			break;
		}

        NS_LOG_UNCOND("node:" << m_currentMobility.id << " to receive message");

		PacketMetadata::ItemIterator metadataIterator = packet->BeginItem();
		PacketMetadata::Item item;
		while (metadataIterator.HasNext()){
            item = metadataIterator.Next();

            // switch case for header type

            m_rxTrace(packet, from);
        }
    }
}

void V2vGeneralClient::AcquireMobilityInfo (void){

    //!< Acquire current mobility stats
    m_currentMobility.ts = Simulator::Now ().GetTimeStep ();
    m_currentMobility.id = this->GetNode ()->GetId ();
    m_currentMobility.position = m_mobilityModel->GetPosition();
    m_currentMobility.velocity = m_mobilityModel->GetVelocity();
    m_currentMobility.direction = m_mobilityModel->GetDirection();
}


void
V2vGeneralClient::StatusReport (void){
    NS_LOG_UNCOND("\n\n-----------------------------------------------------------------------------");
    NS_LOG_UNCOND ("[StatusReport] => At time " << Simulator::Now ().GetSeconds ()
                   << "s node ["<< m_currentMobility.id
        << "] in Cluster: " << m_currentMobility.clusterId
        << " having  ===> \n position: " << m_currentMobility.position << " - Velocity: " << m_currentMobility.velocity
        << " - Direction: " << m_currentMobility.direction
        << "\n last packet sent:" << TimeStep (m_currentMobility.ts).GetSeconds () << "s");
    NS_LOG_UNCOND("----------------------------  2rStableList  ---------------------------------");

    //Print Neighbours Stats
}

void
V2vGeneralClient::HandleAccept (Ptr<Socket> s, const Address& from) {
	NS_LOG_FUNCTION (this << s << from);
    s->SetRecvCallback(MakeCallback(&V2vGeneralClient::HandleRead, this));
}

void
V2vGeneralClient::HandlePeerClose (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}

void
V2vGeneralClient::HandlePeerError (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}



void
V2vGeneralClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &V2vGeneralClient::Send, this);
}

void
V2vGeneralClient::Send (void) {
	NS_LOG_FUNCTION (this);
	NS_ASSERT(m_sendEvent.IsExpired());

    NS_LOG_UNCOND("node:" << m_currentMobility.id << " to send message");

    Ptr<Packet> packet = Create<Packet>(10);
    m_socket->Send (packet);
    // switch case to send different packet types
}

void
V2vGeneralClient::ConnectionSucceeded (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vGeneralClient::ConnectionFailed (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}

} // Namespace ns3
