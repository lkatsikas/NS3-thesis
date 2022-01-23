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

#include <fstream>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/v2v-cluster-header.h"
#include "ns3/v2v-novel-algorithm-client.h"
#include "ns3/v2v-novel-algorithm-helper.h"

#include "ns3/mobility-module.h"
#include "ns3/v2v-mobility-model.h"

using namespace ns3;


/*--------------------------- V2vControlClient Testing ---------------------------*/
class V2vControlClientTestCase: public TestCase {
public:
    V2vControlClientTestCase();
    virtual ~V2vControlClientTestCase();

private:
	virtual void DoRun(void);

};

V2vControlClientTestCase::V2vControlClientTestCase() :
        TestCase("Verify the correct transmission of the packets."){
}

V2vControlClientTestCase::~V2vControlClientTestCase() {
}

void V2vControlClientTestCase::DoRun(void) {

    /*NodeContainer n;
    n.Create (2);

    InternetStackHelper internet;
    internet.Install (n);

    // link the two nodes
    Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
    Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();
    n.Get (0)->AddDevice (txDev);
    n.Get (1)->AddDevice (rxDev);
    Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel> ();
    rxDev->SetChannel (channel1);
    txDev->SetChannel (channel1);
    NetDeviceContainer d;
    d.Add (txDev);
    d.Add (rxDev);


    MobilityHelper ueMobility;
    ueMobility.SetMobilityModel ("ns3::V2vMobilityModel",
         "Mode", StringValue ("Time"),
         "Time", StringValue ("40s"),
         "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=30.0]"),
         "Bounds", RectangleValue (Rectangle (0, 10000, -1000, 1000)));
    ueMobility.Install(n);

    /// Create a 3 line grid of vehicles
    for (uint16_t i = 0; i < 2; i++){
        if(i % 3 == 0){
            n.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 0, 0));
        }
        else if(i % 3 == 1){
            n.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 3, 0));
        }
        else{
            n.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 6, 0));
        }
    }

    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i = ipv4.Assign (d);

    uint16_t port = 4000;

    uint16_t numberOfUes = 10;
    double minimumTdmaSlot = 0.001;         /// Time difference between 2 transmissions
    double clusterTimeMetric = 0.1;         /// Clustering Time Metric for Waiting Time calculation
    double speedVariation = 5.0;

    double tdmaStart = 10*minimumTdmaSlot;
    Ptr<V2vMobilityModel> mobilityModel = n.Get(0)->GetObject<V2vMobilityModel>();
    mobilityModel->SetSpeedVariation(speedVariation);
    V2vNovelAlgorithmClientHelper client1("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetBroadcast(), port)),
            "ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(), port),
            mobilityModel, tdmaStart, numberOfUes, minimumTdmaSlot, clusterTimeMetric);
    //client1.SetAttribute ("MaxPackets", UintegerValue (6));
    ApplicationContainer apps;
    apps.Add(client1.Install(n.Get(0)));

    tdmaStart = 20*minimumTdmaSlot;
    mobilityModel = n.Get(1)->GetObject<V2vMobilityModel>();
    mobilityModel->SetSpeedVariation(speedVariation);
    V2vNovelAlgorithmClientHelper client2("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetBroadcast(), port)),
            "ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(), port),
            mobilityModel, tdmaStart, numberOfUes, minimumTdmaSlot, clusterTimeMetric);
    //client2.SetAttribute ("MaxPackets", UintegerValue (6));
    apps.Add(client2.Install(n.Get(1)));

    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (3.0));

    Simulator::Run ();
    Simulator::Destroy ();

//    NS_TEST_ASSERT_MSG_EQ (client1.GetV2vClient()->GetFormationMessages(), 2, "Vehicle1 Formation messages !");
//    NS_TEST_ASSERT_MSG_EQ (client1.GetV2vClient()->GetMaintenanceMessages(), 0, "Vehicle1 Maintenance messages !");
    */
	Simulator::Run();
    Simulator::Destroy();

}


/*--------------------------- V2vClusterInfoHeader Testing ---------------------------*/
class V2vClusterInfoHeaderTestCase: public TestCase {
public:
    V2vClusterInfoHeaderTestCase();
    virtual ~V2vClusterInfoHeaderTestCase();

private:
	virtual void DoRun(void);

};

V2vClusterInfoHeaderTestCase::V2vClusterInfoHeaderTestCase() :
        TestCase("Check V2vClusterInfoHeader class serialization-deserialization"){
}

V2vClusterInfoHeaderTestCase::~V2vClusterInfoHeaderTestCase() {
}

void V2vClusterInfoHeaderTestCase::DoRun(void) {

    Simulator::Run();
    Simulator::Destroy();
}
/*--------------------------------------------------------------------------*/

/*--------------------------- V2vInitiateClusterHeader Testing ---------------------------*/
class V2vInitiateClusterHeaderTestCase: public TestCase {
public:
    V2vInitiateClusterHeaderTestCase();
    virtual ~V2vInitiateClusterHeaderTestCase();

private:
    virtual void DoRun(void);

};

V2vInitiateClusterHeaderTestCase::V2vInitiateClusterHeaderTestCase() :
        TestCase("Check V2vInitiateClusterHeader class serialization-deserialization"){
}

V2vInitiateClusterHeaderTestCase::~V2vInitiateClusterHeaderTestCase() {
}

void V2vInitiateClusterHeaderTestCase::DoRun(void) {

    Simulator::Run();
    Simulator::Destroy();
}
/*--------------------------------------------------------------------------*/

/*--------------------------- V2vFormClusterHeader Testing ---------------------------*/
class V2vFormClusterHeaderTestCase: public TestCase {
public:
    V2vFormClusterHeaderTestCase();
    virtual ~V2vFormClusterHeaderTestCase();

private:
    virtual void DoRun(void);

};

V2vFormClusterHeaderTestCase::V2vFormClusterHeaderTestCase() :
        TestCase("Check V2vFormClusterHeader class serialization-deserialization"){
}

V2vFormClusterHeaderTestCase::~V2vFormClusterHeaderTestCase() {
}

void V2vFormClusterHeaderTestCase::DoRun(void) {

    Simulator::Run();
    Simulator::Destroy();
}
/*--------------------------------------------------------------------------*/

/*--------------------------- V2vIncidentEventHeader Testing ---------------------------*/
class V2vIncidentEventHeaderTestCase: public TestCase {
public:
    V2vIncidentEventHeaderTestCase();
    virtual ~V2vIncidentEventHeaderTestCase();

private:
    virtual void DoRun(void);

};

V2vIncidentEventHeaderTestCase::V2vIncidentEventHeaderTestCase() :
        TestCase("Check V2vIncidentEventHeader class serialization-deserialization"){
}

V2vIncidentEventHeaderTestCase::~V2vIncidentEventHeaderTestCase() {
}

void V2vIncidentEventHeaderTestCase::DoRun(void) {

    Simulator::Run();
    Simulator::Destroy();
}
/*--------------------------------------------------------------------------*/

/*--------------------------- End-to-end Testing ---------------------------*/
class V2vUdpEndToEndTestCase: public TestCase {
public:
	V2vUdpEndToEndTestCase();
	virtual ~V2vUdpEndToEndTestCase();

private:
	virtual void DoRun(void);

};

V2vUdpEndToEndTestCase::V2vUdpEndToEndTestCase() :
        TestCase("Test an end-to-end clustering scenario."){
}

V2vUdpEndToEndTestCase::~V2vUdpEndToEndTestCase() {
}

void V2vUdpEndToEndTestCase::DoRun(void) {
	// Todo:
}
/*--------------------------------------------------------------------------*/


/*------------------------ TestSuite Initialization ------------------------*/
class V2vTestSuite: public TestSuite {
public:
	V2vTestSuite();
};

V2vTestSuite::V2vTestSuite() : TestSuite("v2v", UNIT) {
    AddTestCase(new V2vControlClientTestCase, TestCase::QUICK);
    AddTestCase(new V2vClusterInfoHeaderTestCase, TestCase::QUICK);
    AddTestCase(new V2vInitiateClusterHeaderTestCase, TestCase::QUICK);
    AddTestCase(new V2vFormClusterHeaderTestCase, TestCase::QUICK);
    AddTestCase(new V2vIncidentEventHeaderTestCase, TestCase::QUICK);
	AddTestCase(new V2vUdpEndToEndTestCase, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static V2vTestSuite v2vTestSuite;

