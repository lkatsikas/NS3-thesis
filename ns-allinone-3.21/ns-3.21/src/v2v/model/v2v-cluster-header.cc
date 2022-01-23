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

#include "ns3/simulator.h"
#include "v2v-cluster-header.h"

NS_LOG_COMPONENT_DEFINE ("V2vClusterHeader");

namespace ns3 {

/* Novel Algorithm Headers */
/////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vNovelCOVHeader);

V2vNovelCOVHeader::V2vNovelCOVHeader() :
        m_ts(Simulator::Now().GetTimeStep()),
        m_tempClusterId(0){
    NS_LOG_FUNCTION (this);
}

V2vNovelCOVHeader::~V2vNovelCOVHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vNovelCOVHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vNovelCOVHeader::SetTempClusterId(uint64_t tempClusterId){
    NS_LOG_FUNCTION (this << tempClusterId);
    m_tempClusterId = tempClusterId;
}

uint64_t
V2vNovelCOVHeader::GetTempClusterId(void) const {
    NS_LOG_FUNCTION (this);
    return m_tempClusterId;
}

TypeId
V2vNovelCOVHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vNovelCOVHeader").SetParent<Header>().AddConstructor<V2vNovelCOVHeader>();
    return tid;
}

TypeId
V2vNovelCOVHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vNovelCOVHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_ts).GetSeconds()
       << " tempClusterId=" << m_tempClusterId <<")";
}

uint32_t
V2vNovelCOVHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(uint64_t);
}

void
V2vNovelCOVHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);
    i.WriteHtonU64(m_tempClusterId);
}

uint32_t
V2vNovelCOVHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64 ();
    m_tempClusterId = i.ReadNtohU64 ();

    return GetSerializedSize();
}

/////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vNovelFormationHeader);

V2vNovelFormationHeader::V2vNovelFormationHeader() :
        m_ts(Simulator::Now().GetTimeStep()),
        m_clusterId(0),
        m_tempClusterId(0){
    NS_LOG_FUNCTION (this);
}

V2vNovelFormationHeader::~V2vNovelFormationHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vNovelFormationHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vNovelFormationHeader::SetClusterId(uint64_t clusterId){
    NS_LOG_FUNCTION (this << clusterId);
    m_clusterId = clusterId;
}

uint64_t
V2vNovelFormationHeader::GetClusterId(void) const {
    NS_LOG_FUNCTION (this);
    return m_clusterId;
}

void
V2vNovelFormationHeader::SetTempClusterId(uint64_t tempClusterId){
    NS_LOG_FUNCTION (this << tempClusterId);
    m_tempClusterId = tempClusterId;
}

uint64_t
V2vNovelFormationHeader::GetTempClusterId(void) const {
    NS_LOG_FUNCTION (this);
    return m_tempClusterId;
}

TypeId
V2vNovelFormationHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vNovelFormationHeader").SetParent<Header>().AddConstructor<V2vNovelFormationHeader>();
    return tid;
}

TypeId
V2vNovelFormationHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vNovelFormationHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os  << "ts=" << m_ts
        << "ClusterId=" << m_clusterId
        << "TempClusterId=" << m_tempClusterId<<")";
}

uint32_t
V2vNovelFormationHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t);
}

void
V2vNovelFormationHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);
    i.WriteHtonU64(m_clusterId);
    i.WriteHtonU64(m_tempClusterId);

}

uint32_t
V2vNovelFormationHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64();
    m_clusterId = i.ReadNtohU64 ();
    m_tempClusterId = i.ReadNtohU64 ();

    return GetSerializedSize();
}

////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vNovelUpdateHeader);

V2vNovelUpdateHeader::V2vNovelUpdateHeader(){
    NS_LOG_FUNCTION (this);
}

V2vNovelUpdateHeader::~V2vNovelUpdateHeader(){
    NS_LOG_FUNCTION (this);
}

void
V2vNovelUpdateHeader::SetUpdateInfo(V2vClusterSap::NovelNeighborInfo updateInfo){
    NS_LOG_FUNCTION (this << updateInfo.id);
    m_updateInfo = updateInfo;
}

V2vClusterSap::NovelNeighborInfo
V2vNovelUpdateHeader::GetUpdateInfo (void) const {
    NS_LOG_FUNCTION (this);
    return m_updateInfo;
}

TypeId
V2vNovelUpdateHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vNovelUpdateHeader").SetParent<Header>().AddConstructor<V2vNovelUpdateHeader>();
    return tid;
}

TypeId
V2vNovelUpdateHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vNovelUpdateHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_updateInfo.ts).GetSeconds()
       << " Id=" << m_updateInfo.id
       << " Cluster Id=" << m_updateInfo.clusterId
       << " CM Members=" << m_updateInfo.chMembers
       << " Position=" << m_updateInfo.position
       << " Velocity=" << m_updateInfo.velocity
       << " Directio=" << m_updateInfo.direction <<")";
}

uint32_t
V2vNovelUpdateHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(V2vClusterSap::NovelNeighborInfo);
}

void
V2vNovelUpdateHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;

    // Write IncidentInfo structure
    unsigned char temp[sizeof(V2vClusterSap::NovelNeighborInfo)];
    memcpy( temp, &m_updateInfo, sizeof(V2vClusterSap::NovelNeighborInfo));
    i.Write(temp, sizeof(V2vClusterSap::NovelNeighborInfo));

}

uint32_t
V2vNovelUpdateHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;

    unsigned char temp[sizeof(V2vClusterSap::NovelNeighborInfo)];
    i.Read(temp, sizeof(V2vClusterSap::NovelNeighborInfo));
    memcpy(&m_updateInfo, &temp, sizeof(V2vClusterSap::NovelNeighborInfo));

    return GetSerializedSize();
}

/////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vNovelMergeHeader);

V2vNovelMergeHeader::V2vNovelMergeHeader() :
        m_ts(Simulator::Now().GetTimeStep()),
        m_oldClusterId(0),
        m_newClusterId(0){
    NS_LOG_FUNCTION (this);
}

V2vNovelMergeHeader::~V2vNovelMergeHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vNovelMergeHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vNovelMergeHeader::SetOldClusterId(uint64_t oldClusterId){
    NS_LOG_FUNCTION (this << oldClusterId);
    m_oldClusterId = oldClusterId;
}

uint64_t
V2vNovelMergeHeader::GetOldClusterId(void) const {
    NS_LOG_FUNCTION (this);
    return m_oldClusterId;
}

void
V2vNovelMergeHeader::SetNewClusterId(uint64_t newClusterId){
    NS_LOG_FUNCTION (this << newClusterId);
    m_newClusterId = newClusterId;
}

uint64_t
V2vNovelMergeHeader::GetNewClusterId(void) const {
    NS_LOG_FUNCTION (this);
    return m_newClusterId;
}

TypeId
V2vNovelMergeHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vNovelMergeHeader").SetParent<Header>().AddConstructor<V2vNovelMergeHeader>();
    return tid;
}

TypeId
V2vNovelMergeHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vNovelMergeHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os  << "ts=" << m_ts
        << "OldClusterId=" << m_oldClusterId
        << "NewClusterId=" << m_newClusterId << ")";
}

uint32_t
V2vNovelMergeHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t);
}

void
V2vNovelMergeHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);
    i.WriteHtonU64(m_oldClusterId);
    i.WriteHtonU64(m_newClusterId);

}

uint32_t
V2vNovelMergeHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64();
    m_oldClusterId = i.ReadNtohU64 ();
    m_newClusterId = i.ReadNtohU64 ();

    return GetSerializedSize();
}


/* Affinity Propagation Headers */
////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vAffinityHelloHeader);


V2vAffinityHelloHeader::V2vAffinityHelloHeader() :
        m_ts(Simulator::Now().GetTimeStep()){
    NS_LOG_FUNCTION (this);
}

V2vAffinityHelloHeader::~V2vAffinityHelloHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vAffinityHelloHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vAffinityHelloHeader::SetHelloInfo(V2vClusterSap::AffinityHello helloInfo){
    NS_LOG_FUNCTION (this << helloInfo.id);
    m_helloInfo = helloInfo;
}

V2vClusterSap::AffinityHello
V2vAffinityHelloHeader::GetHelloInfo (void) const {
    NS_LOG_FUNCTION (this);
    return m_helloInfo;
}

TypeId
V2vAffinityHelloHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vAffinityHelloHeader").SetParent<Header>().AddConstructor<V2vAffinityHelloHeader>();
    return tid;
}

TypeId
V2vAffinityHelloHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vAffinityHelloHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_ts).GetSeconds()
       << " id=" << m_helloInfo.id
       << " CHindex=" << m_helloInfo.CHindex
       << " position=" << m_helloInfo.position
       << " velocity=" << m_helloInfo.velocity
       << " directio=" << m_helloInfo.direction <<")";
}

uint32_t
V2vAffinityHelloHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(V2vClusterSap::AffinityHello);
}

void
V2vAffinityHelloHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);

    // Write IncidentInfo structure
    unsigned char temp[sizeof(V2vClusterSap::AffinityHello)];
    memcpy( temp, &m_helloInfo, sizeof(V2vClusterSap::AffinityHello));
    i.Write(temp, sizeof(V2vClusterSap::AffinityHello));

}

uint32_t
V2vAffinityHelloHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64 ();

    unsigned char temp[sizeof(V2vClusterSap::AffinityHello)];
    i.Read(temp, sizeof(V2vClusterSap::AffinityHello));
    memcpy(&m_helloInfo, &temp, sizeof(V2vClusterSap::AffinityHello));

    return GetSerializedSize();
}

/////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vAffinityRespAvailHeader);

V2vAffinityRespAvailHeader::V2vAffinityRespAvailHeader() :
        m_ts(Simulator::Now().GetTimeStep()){
    NS_LOG_FUNCTION (this);
}

V2vAffinityRespAvailHeader::~V2vAffinityRespAvailHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vAffinityRespAvailHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vAffinityRespAvailHeader::SetRespAvailInfo(V2vClusterSap::AffinityRespAvail respAvailInfo){
    NS_LOG_FUNCTION (this << respAvailInfo.id);
    m_respAvailInfo = respAvailInfo;
}

V2vClusterSap::AffinityRespAvail
V2vAffinityRespAvailHeader::GetRespAvailInfo (void) const {
    NS_LOG_FUNCTION (this);
    return m_respAvailInfo;
}

void
V2vAffinityRespAvailHeader::SetRespAvailList(std::list<V2vClusterSap::AffinitySubStructure> respAvailList){
    NS_LOG_FUNCTION (this << respAvailList.size());
    m_respAvailList = respAvailList;
}

std::list<V2vClusterSap::AffinitySubStructure>
V2vAffinityRespAvailHeader::GetRespAvailList(void) const{
    NS_LOG_FUNCTION (this);
    return m_respAvailList;
}

TypeId
V2vAffinityRespAvailHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vAffinityRespAvailHeader").SetParent<Header>().AddConstructor<V2vAffinityRespAvailHeader>();
    return tid;
}

TypeId
V2vAffinityRespAvailHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vAffinityRespAvailHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_ts).GetSeconds()
       << " id=" << m_respAvailInfo.id
       << " CHindex=" << m_respAvailInfo.CHcnvg
       << " NeighborsNumber=" << m_respAvailInfo.neighboursNumber <<")";
}

uint32_t
V2vAffinityRespAvailHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(V2vClusterSap::AffinityRespAvail)
            + sizeof(V2vClusterSap::AffinitySubStructure)*m_respAvailInfo.neighboursNumber ;
}

void
V2vAffinityRespAvailHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);

    // Write IncidentInfo structure
    unsigned char temp[sizeof(V2vClusterSap::AffinityRespAvail)];
    memcpy( temp, &m_respAvailInfo, sizeof(V2vClusterSap::AffinityRespAvail));
    i.Write(temp, sizeof(V2vClusterSap::AffinityRespAvail));

    for (std::list<V2vClusterSap::AffinitySubStructure>::const_iterator it = m_respAvailList.begin (); it != m_respAvailList.end (); ++it) {
        V2vClusterSap::AffinitySubStructure tmpStruct;
        tmpStruct.id = it->id;
        tmpStruct.resp = it->resp;
        tmpStruct.avail = it->avail;

        unsigned char tmp[sizeof(V2vClusterSap::AffinitySubStructure)];
        memcpy( tmp, &tmpStruct, sizeof(V2vClusterSap::AffinitySubStructure));
        i.Write(tmp, sizeof(V2vClusterSap::AffinitySubStructure));
    }

}

uint32_t
V2vAffinityRespAvailHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64 ();

    unsigned char temp[sizeof(V2vClusterSap::AffinityRespAvail)];
    i.Read(temp, sizeof(V2vClusterSap::AffinityRespAvail));
    memcpy(&m_respAvailInfo, &temp, sizeof(V2vClusterSap::AffinityRespAvail));

    for (uint64_t j = 0; j < m_respAvailInfo.neighboursNumber; ++j) {
        V2vClusterSap::AffinitySubStructure tmpStruct;

        unsigned char tmp[sizeof(V2vClusterSap::AffinitySubStructure)];
        i.Read(tmp, sizeof(V2vClusterSap::AffinitySubStructure));
        memcpy(&tmpStruct, &tmp, sizeof(V2vClusterSap::AffinitySubStructure));

        m_respAvailList.push_back (tmpStruct);
    }

    return GetSerializedSize();
}
////////////////////////////////


/* Modified DMAC Headers */
////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vDMACHelloHeader);


V2vDMACHelloHeader::V2vDMACHelloHeader() :
        m_ts(Simulator::Now().GetTimeStep()){
    NS_LOG_FUNCTION (this);
}

V2vDMACHelloHeader::~V2vDMACHelloHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vDMACHelloHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vDMACHelloHeader::SetHelloInfo(V2vClusterSap::DMACHello helloInfo){
    NS_LOG_FUNCTION (this << helloInfo.id);
    m_helloInfo = helloInfo;
}

V2vClusterSap::DMACHello
V2vDMACHelloHeader::GetHelloInfo (void) const {
    NS_LOG_FUNCTION (this);
    return m_helloInfo;
}

TypeId
V2vDMACHelloHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vDMACHelloHeader").SetParent<Header>().AddConstructor<V2vDMACHelloHeader>();
    return tid;
}

TypeId
V2vDMACHelloHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vDMACHelloHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_ts).GetSeconds()
       << " id=" << m_helloInfo.id
       << " position=" << m_helloInfo.position
       << " velocity=" << m_helloInfo.velocity
       << " directio=" << m_helloInfo.direction
       << ")";
}

uint32_t
V2vDMACHelloHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(V2vClusterSap::DMACHello);
}

void
V2vDMACHelloHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);

    // Write IncidentInfo structure
    unsigned char temp[sizeof(V2vClusterSap::DMACHello)];
    memcpy( temp, &m_helloInfo, sizeof(V2vClusterSap::DMACHello));
    i.Write(temp, sizeof(V2vClusterSap::DMACHello));

}

uint32_t
V2vDMACHelloHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64 ();

    unsigned char temp[sizeof(V2vClusterSap::DMACHello)];
    i.Read(temp, sizeof(V2vClusterSap::DMACHello));
    memcpy(&m_helloInfo, &temp, sizeof(V2vClusterSap::DMACHello));

    return GetSerializedSize();
}


////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vDMACCHHeader);


V2vDMACCHHeader::V2vDMACCHHeader() :
        m_ts(Simulator::Now().GetTimeStep()){
    NS_LOG_FUNCTION (this);
}

V2vDMACCHHeader::~V2vDMACCHHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vDMACCHHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vDMACCHHeader::SetCHInfo(V2vClusterSap::DMACCH chInfo){
    NS_LOG_FUNCTION (this << chInfo.id);
    m_sendCH = chInfo;
}

V2vClusterSap::DMACCH
V2vDMACCHHeader::GetCHInfo (void) const {
    NS_LOG_FUNCTION (this);
    return m_sendCH;
}

TypeId
V2vDMACCHHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vDMACCHHeader").SetParent<Header>().AddConstructor<V2vDMACCHHeader>();
    return tid;
}

TypeId
V2vDMACCHHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vDMACCHHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_ts).GetSeconds()
       << " id=" << m_sendCH.id
       << ")";
}

uint32_t
V2vDMACCHHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(V2vClusterSap::DMACCH);
}

void
V2vDMACCHHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);

    // Write IncidentInfo structure
    unsigned char temp[sizeof(V2vClusterSap::DMACCH)];
    memcpy( temp, &m_sendCH, sizeof(V2vClusterSap::DMACCH));
    i.Write(temp, sizeof(V2vClusterSap::DMACCH));

}

uint32_t
V2vDMACCHHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64 ();

    unsigned char temp[sizeof(V2vClusterSap::DMACCH)];
    i.Read(temp, sizeof(V2vClusterSap::DMACCH));
    memcpy(&m_sendCH, &temp, sizeof(V2vClusterSap::DMACCH));

    return GetSerializedSize();
}


////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vDMACJoinHeader);


V2vDMACJoinHeader::V2vDMACJoinHeader() :
        m_ts(Simulator::Now().GetTimeStep()){
    NS_LOG_FUNCTION (this);
}

V2vDMACJoinHeader::~V2vDMACJoinHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vDMACJoinHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vDMACJoinHeader::SetJoinInfo(V2vClusterSap::DMACJoin joinInfo){
    NS_LOG_FUNCTION (this << joinInfo.id);
    m_sendJoin = joinInfo;
}

V2vClusterSap::DMACJoin
V2vDMACJoinHeader::GetJoinInfo (void) const {
    NS_LOG_FUNCTION (this);
    return m_sendJoin;
}

TypeId
V2vDMACJoinHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vDMACJoinHeader").SetParent<Header>().AddConstructor<V2vDMACJoinHeader>();
    return tid;
}

TypeId
V2vDMACJoinHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vDMACJoinHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_ts).GetSeconds()
       << " id=" << m_sendJoin.id
       << ")";
}

uint32_t
V2vDMACJoinHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(V2vClusterSap::DMACJoin);
}

void
V2vDMACJoinHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);

    // Write IncidentInfo structure
    unsigned char temp[sizeof(V2vClusterSap::DMACJoin)];
    memcpy( temp, &m_sendJoin, sizeof(V2vClusterSap::DMACJoin));
    i.Write(temp, sizeof(V2vClusterSap::DMACJoin));

}

uint32_t
V2vDMACJoinHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64 ();

    unsigned char temp[sizeof(V2vClusterSap::DMACJoin)];
    i.Read(temp, sizeof(V2vClusterSap::DMACJoin));
    memcpy(&m_sendJoin, &temp, sizeof(V2vClusterSap::DMACJoin));

    return GetSerializedSize();
}

} // namespace ns3
