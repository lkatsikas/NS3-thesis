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

#ifndef V2V_CLUSTER_SAP_H
#define V2V_CLUSTER_SAP_H

#include <list>
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {

class V2vClusterSap {

public:
    virtual ~V2vClusterSap ();

    /* Novel Algorithm Structures */
    enum NovelNodeState{
        NONE = 0,
        INITIAL,
        COV,
        FORMATION,
        UPDATE,
        NOVEL_STATES
    };

    enum NovelNodeDegree{
        STANDALONE = 0,
        CH,
        CM,
        DEGREE_STATES
    };

    struct NovelNeighborInfo{
        uint64_t ts;
        uint64_t id;
        uint64_t clusterId;
        uint64_t tempClusterId;
        uint64_t chMembers;

        Vector position;
        Vector velocity;
        Vector direction;
        NovelNodeDegree degree;
    };
    ////////////////////////////////////


    /* Affinity Propagation Structures*/
    enum AffinityNodeState{
        UNDEFINED = 0,
        TH,
        TM,
        TCM,
        AFFINITY_STATES
    };

    struct AffinityCurrentInfo{
        bool CHcnvg;
        uint64_t id;
        uint64_t CHindex;

        Vector position;
        Vector velocity;
        Vector direction;

        double selfSim;
        double selfResp;
        double selfAvail;

        AffinityCurrentInfo():
            id(0),
            CHindex(0),
            position(0.0, 0.0, 0.0),
            velocity(0.0, 0.0, 0.0),
            direction(0.0, 0.0, 0.0),
            selfSim(0.0),
            selfResp(0.0),
            selfAvail(0.0){}
    };

    struct AffinityNeighbors{
        Vector position;
        Vector velocity;
        Vector direction;

        double similarity;
        double availabilitySent;
        double availabilityReceived;
        double responsibilitySent;
        double responsibilityReceived;

        bool CHcnvg;
        Time tExpire;
        uint64_t CHindex;

        AffinityNeighbors():
            position(0.0, 0.0, 0.0),
            velocity(0.0, 0.0, 0.0),
            direction(0.0, 0.0, 0.0),
            similarity(0.0),
            availabilitySent(0.0),
            availabilityReceived(0.0),
            responsibilitySent(0.0),
            responsibilityReceived(0.0),
            CHcnvg(false),
            CHindex(0){}
    };

    struct AffinityHello{
        uint64_t id;
        uint64_t CHindex;

        Vector position;
        Vector velocity;
        Vector direction;

        AffinityHello():
            id(0),
            CHindex(0),
            position(0.0, 0.0, 0.0),
            velocity(0.0, 0.0, 0.0),
            direction(0.0, 0.0, 0.0){}
    };

    struct AffinitySubStructure{
        uint64_t id;
        double resp;
        double avail;

        AffinitySubStructure():
            id(0),
            resp(0.0),
            avail(0.0){}
    };

    struct AffinityRespAvail{
        uint64_t id;
        bool CHcnvg;
        uint64_t neighboursNumber;

        AffinityRespAvail():
            id(0),
            CHcnvg(false),
            neighboursNumber(0){}
    };
    ////////////////////////////////////


    /* Modified DMAC Structures*/
    enum DMACNodeState{
        INIT,
        SENDJOIN,
        SENDCH,
        FORWARDHELLO,
        FORWARDCH,
        FORWARDJOIN,
        DMAC_STATES
    };

    struct DMACCurrentInfo{
        uint64_t id;
        double weight;
        uint64_t CHindex;

        Vector position;
        Vector velocity;
        Vector direction;

        NovelNodeDegree role;

        DMACCurrentInfo():
            id(0.0),
            weight(0.0),
            CHindex(0),
            position(0.0, 0.0, 0.0),
            velocity(0.0, 0.0, 0.0),
            direction(0.0, 0.0, 0.0){}
    };

    struct DMACNeighbours{
        double beats;
        double weight;

        Vector position;
        Vector velocity;
        Vector direction;

        Time tExpire;
        NovelNodeDegree role;

        DMACNeighbours():
            beats(0.0),
            weight(0.0),
            position(0.0, 0.0, 0.0),
            velocity(0.0, 0.0, 0.0),
            direction(0.0, 0.0, 0.0){}
    };

    struct DMACHello{
        uint64_t id;
        uint16_t ttl;
        double weight;

        Vector position;
        Vector velocity;
        Vector direction;

        NovelNodeDegree role;

        DMACHello():
            id(0),
            weight(0.0),
            position(0.0, 0.0, 0.0),
            velocity(0.0, 0.0, 0.0),
            direction(0.0, 0.0, 0.0){}
    };

    struct DMACCH{
        uint64_t id;
        uint16_t ttl;
    };

    struct DMACJoin{
        uint64_t id;
        uint16_t ttl;
        uint64_t CHindex;
    };

private:

};

} // namespace ns3

#endif // V2V_CLUSTER_SAP_H

