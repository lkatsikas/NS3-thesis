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

#include "ns3/names.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/v2v-modified-dmac-algorithm-helper.h"


namespace ns3 {

V2vModifiedDMACAlgorithmHelper::V2vModifiedDMACAlgorithmHelper(std::string protocolLocal, Address addressLocal,
    std::string protocolListeningLocal, Address addressListeningLocal,
    Ptr<MobilityModel> mobilityModel)
{
  m_factory.SetTypeId ("ns3::V2vModifiedDMACAlgorithmClient");
  m_factory.Set ("ProtocolSendingLocal", StringValue (protocolLocal));
  m_factory.Set ("SendingLocal", AddressValue (addressLocal));
  m_factory.Set ("ProtocolListeningLocal", StringValue (protocolListeningLocal));
  m_factory.Set ("ListeningLocal", AddressValue (addressListeningLocal));

  m_factory.Set ("MobilityModel", PointerValue (mobilityModel));
}

void
V2vModifiedDMACAlgorithmHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
V2vModifiedDMACAlgorithmHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
V2vModifiedDMACAlgorithmHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
V2vModifiedDMACAlgorithmHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
V2vModifiedDMACAlgorithmHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<V2vModifiedDMACAlgorithmClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
