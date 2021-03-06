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

#include "ns3/core-module.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/network-module.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/v2v-mobility-model.h"
#include "ns3/v2v-general-helper.h"


#define SIMULATION_TIME_FORMAT(s) Seconds(s)


using namespace ns3;
NS_LOG_COMPONENT_DEFINE("V2vGeneralExample");


int main(int argc, char *argv[]) {

    /*--------------------- Logging System Configuration -------------------*/
    LogLevel logLevel = (LogLevel) (LOG_PREFIX_ALL | LOG_LEVEL_ALL);
    LogComponentEnable("V2vGeneralExample", logLevel);
    LogComponentEnable("V2vGeneralClient", logLevel);

    NS_LOG_UNCOND("/------------------------------------------------\\");
    NS_LOG_UNCOND(" - A generic client for VANETs [Example] -> Cluster vehicles communication");
    NS_LOG_UNCOND("\\------------------------------------------------/");
    /*----------------------------------------------------------------------*/

    /*---------------------- Simulation Default Values ---------------------*/
    std::string phyMode ("OfdmRate6MbpsBW10MHz");

    uint16_t numberOfUes = 10;
    double minimumTdmaSlot = 0.001;         /// Time difference between 2 transmissions
    double speedVariation = 5.0;

    double simTime = 750.0;

    std::string range;
    int gain;
    int power;
    /*----------------------------------------------------------------------*/


    /*-------------------- Set explicitly default values -------------------*/
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                            StringValue ("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                            StringValue ("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                            StringValue (phyMode));
    /*----------------------------------------------------------------------*/


    /*-------------------- Command Line Argument Values --------------------*/
    CommandLine cmd;
    cmd.AddValue("ueNumber", "Number of UE", numberOfUes);
    cmd.AddValue("simTime", "Simulation Time in Seconds", simTime);
    cmd.AddValue("range", "Transmission range of the vehicles", range);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("");
    NS_LOG_INFO("|---"<< " SimTime -> " << simTime <<" ---|\n");
    NS_LOG_INFO("|---"<< " Number of UE -> " << numberOfUes <<" ---|\n");

    if(strcasecmp ((char*)range.c_str (), "High") == 0){
        NS_LOG_INFO("High transimssion range set.");
        power = 32;
        gain = 12;
    }
    else if(strcasecmp ((char*)range.c_str (), "Medium") == 0){
        NS_LOG_INFO("Medium transimssion range set.");
        power = 28;
        gain = 9;
    }
    else if(strcasecmp ((char*)range.c_str (), "Low") == 0){
        NS_LOG_INFO("Low transimssion range set.");
        power = 24;
        gain = 6;
    }
    else{
        std::cout << "Invalid transmission range. High/Medium/Low are supported options.";
        return 0;
    }
    /*----------------------------------------------------------------------*/


    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    /*----------------------------------------------------------------------*/


    /*------------------------- Create UEs-EnodeBs -------------------------*/
    NodeContainer ueNodes;
    ueNodes.Create(numberOfUes);
    /*----------------------------------------------------------------------*/


    /*-------------------- Install Mobility Model in Ue --------------------*/
    MobilityHelper ueMobility;
    ueMobility.SetMobilityModel ("ns3::V2vMobilityModel",
         "Mode", StringValue ("Time"),
         "Time", StringValue ("40s"),
         "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=30.0]"),
         "Bounds", RectangleValue (Rectangle (0, 10000, -1000, 1000)));
    ueMobility.Install(ueNodes);

    /// Create a 3 line grid of vehicles
    for (uint16_t i = 0; i < numberOfUes; i++){
        if(i % 3 == 0){
            ueNodes.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 0, 0));
        }
        else if(i % 3 == 1){
            ueNodes.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 3, 0));
        }
        else{
            ueNodes.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (i*5, 6, 0));
        }
    }
    /*----------------------------------------------------------------------*/


    /*------------------- Install the IP stack on the UEs ------------------*/
    InternetStackHelper internet;
    internet.Install(ueNodes);
    /*----------------------------------------------------------------------*/


    /*-------------------------- Setup Wifi nodes --------------------------*/
    // The below set of helpers will help us to put together the wifi NICs we want
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    Ptr<YansWifiChannel> channel = wifiChannel.Create ();

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    wifiPhy.SetChannel (channel);
    wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
    wifiPhy.Set ("TxPowerStart", DoubleValue(power));
    wifiPhy.Set ("TxPowerEnd", DoubleValue(power));
    wifiPhy.Set ("TxGain", DoubleValue(gain));
    wifiPhy.Set ("RxGain", DoubleValue(gain));
    wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(-61.8));
    wifiPhy.Set ("CcaMode1Threshold", DoubleValue(-64.8));

    NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
    Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
    //wifi80211p.EnableLogComponents ();

    wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));
    NetDeviceContainer wifiDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, ueNodes);

    NS_LOG_INFO ("Assign IP Addresses.");
    ipv4h.SetBase ("10.1.1.0", "255.255.255.0");
    ipv4h.Assign (wifiDevices);

    uint16_t controlPort = 3999;
    ApplicationContainer controlApps;

    /**
     * Setting Generic Client
     */
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        //!< Initial TDMA UE synchronization Function
        double vehicleTdmaSlot = (u+1)*minimumTdmaSlot;

        Ptr<V2vMobilityModel> mobilityModel = ueNodes.Get(u)->GetObject<V2vMobilityModel>();
        mobilityModel->SetSpeedVariation(speedVariation);
        V2vGeneralHelper ueClient("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetBroadcast(), controlPort)),
                "ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(), controlPort), mobilityModel);
        ueClient.SetAttribute ("MaxUes", UintegerValue(numberOfUes));
        ueClient.SetAttribute ("MinimumTdmaSlot", DoubleValue(minimumTdmaSlot));
        ueClient.SetAttribute ("VehicleTdmaSlot", DoubleValue(vehicleTdmaSlot));
        controlApps.Add(ueClient.Install(ueNodes.Get(u)));
    }

    controlApps.Start (Seconds(0.));
    controlApps.Stop (Seconds(simTime-0.1));

    AsciiTraceHelper ascii;
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream ("src/v2v/examples/output/socket-options-ipv4.txt"));
    wifiPhy.EnablePcapAll ("src/v2v/examples/output/socket.pcap", false);

    /*----------------------------------------------------------------------*/


    /*---------------------- Simulation Stopping Time ----------------------*/
    Simulator::Stop(SIMULATION_TIME_FORMAT(simTime));
    /*----------------------------------------------------------------------*/

    /*--------------------------- Simulation Run ---------------------------*/
    Simulator::Run();
    Simulator::Destroy();
    /*----------------------------------------------------------------------*/

    return EXIT_SUCCESS;
}
