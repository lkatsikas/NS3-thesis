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

#ifndef V2V_CLUSTER_HEADER_H
#define V2V_CLUSTER_HEADER_H

#include "ns3/log.h"
#include "ns3/header.h"
#include "v2v-cluster-sap.h"

namespace ns3 {

/* Novel Algorithm Headers */
/**
 * \ingroup v2v
 * \class V2vNovelCOVHeader
 * \brief Packet header for V2vNovelClient application.
 *
 * The header is made of a 64bits temporary cluster ID followed by
 * a 64bits time stamp.
 */
class V2vNovelCOVHeader: public Header {
public:

    V2vNovelCOVHeader();
    virtual ~V2vNovelCOVHeader();

    /**
     * \return the time stamp
     */
    Time GetTs(void) const;

    /**
     * \param degree the degree of the node
     */
    void SetTempClusterId(uint64_t tempClusterId);
    /**
     * \return the degree of the node
     */
    uint64_t GetTempClusterId(void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    uint64_t m_ts;                  //!< Timestamp
    uint64_t m_tempClusterId;       //!< Temporary Cluster id
};

/**
 * \ingroup v2v
 * \class V2vNovelFormationHeader
 * \brief Packet header for V2vControlClient application.
 *
 * The header is made of a 64bits cluster ID followed by
 * a 64bits time stamp.
 */
class V2vNovelFormationHeader: public Header {
public:

    V2vNovelFormationHeader();
    virtual ~V2vNovelFormationHeader();

    /**
     * \return the time stamp
     */
    Time GetTs(void) const;

    /**
     * \param degree the degree of the node
     */
    void SetClusterId(uint64_t clusterId);
    /**
     * \return the degree of the node
     */
    uint64_t GetClusterId(void) const;

    /**
     * \param degree the degree of the node
     */
    void SetTempClusterId(uint64_t tempClusterId);
    /**
     * \return the degree of the node
     */
    uint64_t GetTempClusterId(void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    uint64_t m_ts;
    uint64_t m_clusterId;
    uint64_t m_tempClusterId;
};

/**
 * \ingroup v2v
 * \class V2vNovelUpdateHeader
 * \brief Packet header for V2vNovelClient application.
 *
 */
class V2vNovelUpdateHeader: public Header {
public:

    V2vNovelUpdateHeader();
    virtual ~V2vNovelUpdateHeader();

    /**
     * \param NovelNeighborInfo structure
     */
    void SetUpdateInfo(V2vClusterSap::NovelNeighborInfo updateInfo);
    /**
     * \return the NovelNeighborInfo struct
     */
    V2vClusterSap::NovelNeighborInfo GetUpdateInfo(void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    V2vClusterSap::NovelNeighborInfo m_updateInfo;
};

/**
 * \ingroup v2v
 * \class V2vNovelMergeHeader
 * \brief Packet header for V2vControlClient application.
 *
 * The header is made of a 64bits cluster ID followed by
 * a 64bits time stamp.
 */
class V2vNovelMergeHeader: public Header {
public:

    V2vNovelMergeHeader();
    virtual ~V2vNovelMergeHeader();

    /**
     * \return the time stamp
     */
    Time GetTs(void) const;

    /**
     * \param oldClusterId the old cluster id
     */
    void SetOldClusterId(uint64_t oldClusterId);

    /**
     * \return the old cluster id
     */
    uint64_t GetOldClusterId(void) const;

    /**
     * \param newClusterId the new cluster id
     */
    void SetNewClusterId(uint64_t newClusterId);

    /**
     * \return the new cluster id
     */
    uint64_t GetNewClusterId(void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    uint64_t m_ts;
    uint64_t m_oldClusterId;
    uint64_t m_newClusterId;
};


/* Affinity Propagation Headers */
/**
 * \ingroup v2v
 * \class V2vAffinityHelloHeader
 * \brief Packet header for V2vAffinityAlgorithmClient application.
 *
 */
class V2vAffinityHelloHeader: public Header {
public:

    V2vAffinityHelloHeader();
    virtual ~V2vAffinityHelloHeader();

    /**
     * \return the time stamp
     */
    Time GetTs(void) const;

    /**
     * \param AffinityHello structure
     */
    void SetHelloInfo(V2vClusterSap::AffinityHello helloInfo);
    /**
     * \return the V2vClusterSap::AffinityHello struct
     */
    V2vClusterSap::AffinityHello GetHelloInfo(void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    uint64_t m_ts;                                      //!< Timestamp
    V2vClusterSap::AffinityHello m_helloInfo;
};

/**
 * \ingroup v2v
 * \class V2vAffinityRespAvailHeader
 * \brief Packet header for V2vAffinityAlgorithmClient application.
 *
 */
class V2vAffinityRespAvailHeader: public Header {
public:

    V2vAffinityRespAvailHeader();
    virtual ~V2vAffinityRespAvailHeader();

    /**
     * \return the time stamp
     */
    Time GetTs(void) const;

    /**
     * \param AffinityRespAvail structure
     */
    void SetRespAvailInfo(V2vClusterSap::AffinityRespAvail respAvailInfo);
    /**
     * \return the V2vClusterSap::AffinityRespAvail struct
     */
    V2vClusterSap::AffinityRespAvail GetRespAvailInfo(void) const;

    /**
     * @brief SetRespAvailList
     * @param respAvailList
     */
    void SetRespAvailList(std::list<V2vClusterSap::AffinitySubStructure> respAvailList);

    /**
     * @brief GetRespAvailList
     * @return
     */
    std::list<V2vClusterSap::AffinitySubStructure> GetRespAvailList(void) const;


    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    uint64_t m_ts;                                      //!< Timestamp
    V2vClusterSap::AffinityRespAvail m_respAvailInfo;
    std::list<V2vClusterSap::AffinitySubStructure> m_respAvailList;
};
//////////////////////////////////



/* Modified DMAC Headers */
/**
 * \ingroup v2v
 * \class V2vDMACHelloHeader
 * \brief Packet header for V2vModifiedDMACAlgorithmClient application.
 *
 */
class V2vDMACHelloHeader: public Header {
public:

    V2vDMACHelloHeader();
    virtual ~V2vDMACHelloHeader();

    /**
     * \return the time stamp
     */
    Time GetTs(void) const;

    /**
     * \param DMACHello structure
     */
    void SetHelloInfo(V2vClusterSap::DMACHello helloInfo);
    /**
     * \return the V2vClusterSap::DMACHello struct
     */
    V2vClusterSap::DMACHello GetHelloInfo(void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    uint64_t m_ts;                                      //!< Timestamp
    V2vClusterSap::DMACHello m_helloInfo;
};


/**
 * \ingroup v2v
 * \class V2vDMACCHHeader
 * \brief Packet header for V2vModifiedDMACAlgorithmClient application.
 *
 */
class V2vDMACCHHeader: public Header {
public:

    V2vDMACCHHeader();
    virtual ~V2vDMACCHHeader();

    /**
     * \return the time stamp
     */
    Time GetTs(void) const;

    /**
     * \param DMACCH structure
     */
    void SetCHInfo(V2vClusterSap::DMACCH chInfo);
    /**
     * \return the V2vClusterSap::DMACCH struct
     */
    V2vClusterSap::DMACCH GetCHInfo(void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    uint64_t m_ts;                                      //!< Timestamp
    V2vClusterSap::DMACCH m_sendCH;
};


/**
 * \ingroup v2v
 * \class V2vDMACJoinHeader
 * \brief Packet header for V2vModifiedDMACAlgorithmClient application.
 *
 */
class V2vDMACJoinHeader: public Header {
public:

    V2vDMACJoinHeader();
    virtual ~V2vDMACJoinHeader();

    /**
     * \return the time stamp
     */
    Time GetTs(void) const;

    /**
     * \param DMACJOIN structure
     */
    void SetJoinInfo(V2vClusterSap::DMACJoin joinInfo);
    /**
     * \return the V2vClusterSap::DMACJoin struct
     */
    V2vClusterSap::DMACJoin GetJoinInfo(void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:

    uint64_t m_ts;                                      //!< Timestamp
    V2vClusterSap::DMACJoin m_sendJoin;
};


} // namespace ns3

#endif // V2V_CLUSTER_HEADER_H

