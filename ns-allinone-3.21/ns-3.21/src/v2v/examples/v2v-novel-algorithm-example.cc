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
#include "ns3/stats-module.h"

#include "ns3/v2v-mobility-model.h"
#include "ns3/v2v-novel-algorithm-helper.h"


#define SIMULATION_TIME_FORMAT(s) Seconds(s)


using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE("V2vNovelAlgorithmExample");

void printNumberofClusterMembers(NodeContainer ueNodes){

    double sum = 0.0;
    uint64_t chs = 0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        double t = tmp->GetClusterMembers();
        if(t > 0){
            sum += t;
            chs ++;
        }

    }

    NS_LOG_UNCOND("Average number of members in clusters: " << sum/chs);
}

void printFormationDelay(NodeContainer ueNodes){

    double sum = 0.0;
    uint64_t del = 0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        sum += tmp->GetFormationDelay();
        del ++;
    }

    NS_LOG_UNCOND("Average Formation Delay: " << sum/del);
}

void printClusterChanges(NodeContainer ueNodes){

    double sum = 0.0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        sum += tmp->GetClusterChanges();
    }

    NS_LOG_UNCOND("Total Cluster Changes: " << sum);
    NS_LOG_UNCOND("Average Number of Cluster Changes: " << sum/ueNodes.GetN ());
}

void printNumberofMessages(NodeContainer ueNodes, double simTime, double trainingPeriod){

    double sum = 0.0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        sum += tmp->GetNumberOfMessages();
    }

    NS_LOG_UNCOND("Average Number of messages: " << sum/ueNodes.GetN ());
    NS_LOG_UNCOND("Average Number of messages Per Second: " << (sum/ueNodes.GetN ()) / (simTime-trainingPeriod));
}

void calculateClustersNumber(NodeContainer ueNodes, Ptr<FileAggregator> aggregator, std::string datasetContext, double trainingPeriod){

    // aggregator must be turned on
    aggregator->Enable ();

    uint64_t sum = 0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        if(tmp->GetRole() == V2vClusterSap::CH){
            sum ++;
        }
    }

    aggregator->Write2d (datasetContext, Simulator::Now ().GetSeconds () - trainingPeriod, sum);

    // Disable logging of data for the aggregator.
    aggregator->Disable ();

    Simulator::Schedule(Seconds(1.0),&calculateClustersNumber, ueNodes, aggregator, datasetContext, trainingPeriod);
}

void calculateNumberofMessagesPerSecond(NodeContainer ueNodes, Ptr<FileAggregator> aggregator, std::string datasetContext, double trainingPeriod){

    // aggregator must be turned on
    aggregator->Enable ();

    double sum = 0.0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        sum += tmp->GetNumberOfMessagesPerSecond();
    }

    aggregator->Write2d (datasetContext, Simulator::Now ().GetSeconds () - trainingPeriod, sum);

    // Disable logging of data for the aggregator.
    aggregator->Disable ();

    Simulator::Schedule(Seconds(1.0),&calculateNumberofMessagesPerSecond, ueNodes, aggregator, datasetContext, trainingPeriod);
}

void calculateClusterChangesPerSecond(NodeContainer ueNodes, Ptr<FileAggregator> aggregator, std::string datasetContext, double trainingPeriod){

    // aggregator must be turned on
    aggregator->Enable ();

    double sum = 0.0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        sum += tmp->GetClusterChangesPerSecond();
    }

    aggregator->Write2d (datasetContext, Simulator::Now ().GetSeconds () - trainingPeriod, sum);

    // Disable logging of data for the aggregator.
    aggregator->Disable ();

    Simulator::Schedule(Seconds(1.0),&calculateClusterChangesPerSecond, ueNodes, aggregator, datasetContext, trainingPeriod);
}

void printChDuration(NodeContainer ueNodes){

    double sum = 0.0;
    uint64_t del = 0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        sum += tmp->GetChDuration();
        del ++;
    }

    NS_LOG_UNCOND("Average CH duration is: " << sum/del);
}

void printCmDuration(NodeContainer ueNodes){

    double sum = 0.0;
    uint64_t del = 0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();

        sum += tmp->GetCmDuration();
        del ++;
    }

    NS_LOG_UNCOND("Average CM duration is: " << sum/del);
}

void printMinChDuration(NodeContainer ueNodes){

    double min = 100000.0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();
        double t = tmp->GetMinChDuration();
        if(t < min){
            min = t;
        }
    }
    NS_LOG_UNCOND("Min CH duration is: " << min);
}

void printMinCmDuration(NodeContainer ueNodes){

    double min = 100000.0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();
        double t = tmp->GetMinCmDuration();
        if(t < min){
            min = t;
        }
    }
    NS_LOG_UNCOND("Min CM duration is: " << min);
}

void printMaxChDuration(NodeContainer ueNodes){

    double max = 0.0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();
        double t = tmp->GetMaxChDuration();
        if(t > max){
            max = t;
        }
    }
    NS_LOG_UNCOND("Max CH duration is: " << max);
}

void printMaxCmDuration(NodeContainer ueNodes){

    double max = 0.0;
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        Ptr<Application> temp = ueNodes.Get(u)->GetApplication (0);
        Ptr<V2vNovelAlgorithmClient> tmp = temp->GetObject<V2vNovelAlgorithmClient>();
        double t = tmp->GetMaxCmDuration();
        if(t > max){
            max = t;
        }
    }
    NS_LOG_UNCOND("Max CM duration is: " << max);
}

int main(int argc, char *argv[]) {

    /*--------------------- Logging System Configuration -------------------*/
    LogLevel logLevel = (LogLevel) (LOG_PREFIX_ALL | LOG_LEVEL_WARN);
    LogComponentEnable("V2vNovelAlgorithmExample", logLevel);
    LogComponentEnable("V2vNovelAlgorithmClient", logLevel);
    //LogComponentEnable ("Ns2MobilityHelper",logLevel);

    std::string exampleName ("V2vNovelAlgorithmClient");

    NS_LOG_UNCOND("/--------------------------------------------------------------------------\\");
    NS_LOG_UNCOND(" - A novel algorithm for VANETs [Example] -> Cluster vehicles communication");
    NS_LOG_UNCOND("\\--------------------------------------------------------------------------/");
    /*----------------------------------------------------------------------*/

    /*---------------------- Simulation Default Values ---------------------*/
    std::string phyMode ("OfdmRate6MbpsBW10MHz");

    uint16_t numberOfUes = 10;

    double clusterTimeMetric = 1.0;         /// Clustering Time Metric for Waiting Time calculation
    double minimumTdmaSlot = 0.01;         /// Time difference between 2 transmissions

    double simTime = 750.0;
    double trainingPeriod = 80.0;


    std::string traceFile;
    std::string logFile;
    std::string range;
    std::string scenario;
    double gain;
    double power;
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
    cmd.AddValue("training", "Training period of Time", trainingPeriod);


    cmd.AddValue ("traceFile", "Ns3 movement trace file", traceFile);
    cmd.AddValue ("logFile", "Log file", logFile);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("");
    NS_LOG_INFO("|---"<< " SimTime -> " << simTime <<" ---|\n");
    NS_LOG_INFO("|---"<< " Number of UE -> " << numberOfUes <<" ---|\n");
    NS_LOG_INFO("|---"<< " Transmission Range -> " << range <<" ---|\n");


    if (traceFile.empty () || numberOfUes <= 0 || simTime <= 0 || logFile.empty ())
      {
        std::cout << "Usage of " << argv[0] << " :\n\n"
        "./waf --run \"ns2-mobility-trace"
        " --traceFile=src/mobility/examples/default.ns_movements"
        " --nodeNum=2 --duration=100.0 --logFile=ns2-mob.log\" \n\n"
        "NOTE: ns2-traces-file could be an absolute or relative path. You could use the file default.ns_movements\n"
        "      included in the same directory of this example file.\n\n"
        "NOTE 2: Number of nodes present in the trace file must match with the command line argument and must\n"
        "        be a positive number. Note that you must know it before to be able to load it.\n\n"
        "NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";

        return 0;
      }

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

    if(traceFile.find ("Scenario1") != string::npos){
        scenario = "Scenario1";
    }
    else if(traceFile.find ("Scenario2") != string::npos){
        scenario = "Scenario2";
    }
    else if(traceFile.find ("Scenario3") != string::npos){
        scenario = "Scenario3";
    }
    else if(traceFile.find ("KarradaIn") != string::npos){
        scenario = "KarradaIn";
    }
    else if(traceFile.find ("Batavia") != string::npos){
        scenario = "Batavia";
    }
    else if(traceFile.find ("Manhattan") != string::npos){
        scenario = "Manhattan";
    }
    else{
        std::cout << "Invalid Scenario given in tracefile";
        return 0;
    }

    if(trainingPeriod < 0){
        std::cout << "Training period could not be negative";
        return 0;
    }


    // Create Ns2MobilityHelper with the specified trace log file as parameter
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);

    // open log file for output
    std::ofstream os;
    os.open (logFile.c_str ());
    /*----------------------------------------------------------------------*/


    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    /*----------------------------------------------------------------------*/


    /*------------------------- Create UEs-EnodeBs -------------------------*/
    NodeContainer dummyNode;
    dummyNode.Create(1);
    NodeContainer ueNodes;
    ueNodes.Create(numberOfUes);

    ns2.Install (ueNodes.Begin (), ueNodes.End ());
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
    wifiPhy.Set ("TxPowerLevels", UintegerValue(1));
    wifiPhy.Set ("TxGain", DoubleValue(gain));
    wifiPhy.Set ("RxGain", DoubleValue(gain));
    wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(-71.8));
    wifiPhy.Set ("CcaMode1Threshold", DoubleValue(-74.8));

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
     * Setting Control Channel
     */
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {

        //!< Initial TDMA UE synchronization Function
        double vehicleTdmaSlot = (u+1)*minimumTdmaSlot;

        Ptr<MobilityModel> mobilityModel = ueNodes.Get(u)->GetObject<MobilityModel>();
        V2vNovelAlgorithmHelper ueClient("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetBroadcast(), controlPort)),
                "ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(), controlPort), mobilityModel);

        ueClient.SetAttribute ("TrainingPeriod", DoubleValue(trainingPeriod));
        ueClient.SetAttribute ("MaxUes", UintegerValue(numberOfUes));
        ueClient.SetAttribute ("MinimumTdmaSlot", DoubleValue(minimumTdmaSlot));
        ueClient.SetAttribute ("VehicleTdmaSlot", DoubleValue(vehicleTdmaSlot));
        ueClient.SetAttribute ("ClusterTimeMetric", DoubleValue(clusterTimeMetric));
        controlApps.Add(ueClient.Install(ueNodes.Get(u)));
    }

    controlApps.Start (Seconds(0.));
    controlApps.Stop (Seconds(simTime-0.1));

    AsciiTraceHelper ascii;
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream ("src/v2v/examples/output/socket-options-ipv4.txt"));
    wifiPhy.EnablePcapAll ("src/v2v/examples/output/socket.pcap", false);
    /*----------------------------------------------------------------------*/

    string numberofClustersFile       = "NumberOfClusters[" + exampleName + scenario + range + "].txt";
    string datasetContext = "Dataset/Context/String";

    Ptr<FileAggregator> aggregator1 = CreateObject<FileAggregator> (numberofClustersFile, FileAggregator::FORMATTED);
    aggregator1->Set2dFormat ("%.3e\t%.0f");

    Simulator::Schedule(Seconds(simTime), printNumberofClusterMembers, ueNodes);
    Simulator::Schedule(Seconds(simTime), printFormationDelay, ueNodes);
    Simulator::Schedule(Seconds(simTime), printClusterChanges, ueNodes);
    Simulator::Schedule(Seconds(simTime), printNumberofMessages, ueNodes, simTime, trainingPeriod);
    Simulator::Schedule(Seconds(trainingPeriod), calculateClustersNumber, ueNodes, aggregator1, datasetContext, trainingPeriod);

    string numberofMessagesPerSecondFile       = "NumberOfMessagesPerSecond[" + exampleName + scenario + range + "].txt";
    Ptr<FileAggregator> aggregator2 = CreateObject<FileAggregator> (numberofMessagesPerSecondFile, FileAggregator::FORMATTED);
    aggregator2->Set2dFormat ("%.3e\t%.0f");
    Simulator::Schedule(Seconds(trainingPeriod), calculateNumberofMessagesPerSecond, ueNodes, aggregator2, datasetContext, trainingPeriod);

    string clusterChangesPerSecondFile       = "ClusterChangesPerSecond[" + exampleName + scenario + range + "].txt";
    Ptr<FileAggregator> aggregator3 = CreateObject<FileAggregator> (clusterChangesPerSecondFile, FileAggregator::FORMATTED);
    aggregator3->Set2dFormat ("%.3e\t%.0f");
    Simulator::Schedule(Seconds(trainingPeriod), calculateClusterChangesPerSecond, ueNodes, aggregator3, datasetContext, trainingPeriod);

    Simulator::Schedule(Seconds(simTime), printChDuration, ueNodes);
    Simulator::Schedule(Seconds(simTime), printCmDuration, ueNodes);
    Simulator::Schedule(Seconds(simTime), printMinChDuration, ueNodes);
    Simulator::Schedule(Seconds(simTime), printMaxChDuration, ueNodes);
    Simulator::Schedule(Seconds(simTime), printMinCmDuration, ueNodes);
    Simulator::Schedule(Seconds(simTime), printMaxCmDuration, ueNodes);

    /*---------------------- Simulation Stopping Time ----------------------*/
    Simulator::Stop(SIMULATION_TIME_FORMAT(simTime));
    /*----------------------------------------------------------------------*/

    /*--------------------------- Simulation Run ---------------------------*/
    Simulator::Run();
    Simulator::Destroy();
    /*----------------------------------------------------------------------*/

    return EXIT_SUCCESS;
}
