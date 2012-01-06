/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
 * phone: ++49-241-80-27910,
 * fax: ++49-241-80-22242
 * email: info@openwns.org
 * www: http://www.openwns.org
 * _____________________________________________________________________________
 *
 * openWNS is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 2 as published by the
 * Free Software Foundation;
 *
 * openWNS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <LTE/helper/idprovider/CellId.hpp>
#include <WNS/service/dll/StationTypes.hpp>

using namespace lte::helper::idprovider;

CellId::CellId(dll::Layer2* layer2, std::string modeName) :
    layer2_(layer2),
    modeName_(modeName),
    key_("MAC.CellId"),
    associationService_(NULL),
    isBS_(false)
{
    if(layer2_->getStationType() == wns::service::dll::StationTypes::eNB())
        isBS_ = true;
    else
        associationService_ = layer2_->getControlService<dll::services::control::Association>(
            "ASSOCIATION"+modeName);
}


CellId::~CellId()
{
}

void
CellId::doVisit(wns::probe::bus::IContext& c) const
{
    int id = -1;
    if(isBS_)
        id = layer2_->getDLLAddress().getInteger();
    else
    {
        assure(associationService_, "Need AssocService in UE");

        if (associationService_->hasAssociation())
            id = associationService_->getAssociation().getInteger();
    }    

    c.insertInt(key_, id);
}


