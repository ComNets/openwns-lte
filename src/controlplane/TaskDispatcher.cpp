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

#include <LTE/controlplane/TaskDispatcher.hpp>
#include <WNS/Assure.hpp>

using namespace lte::controlplane;

STATIC_FACTORY_REGISTER_WITH_CREATOR(NamedDispatcher,
									 wns::ldk::FunctionalUnit,
									 "lte.macr.NamedDispatcher",
									 wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(TaskDispatcherIncoming,
									 wns::ldk::FunctionalUnit,
									 "lte.macr.TaskDispatcherIncoming",
									 wns::ldk::FUNConfigCreator);



wns::ldk::IDelivererReceptacle*
TaskDeliverer::getAcceptor(const wns::ldk::CompoundPtr& compound)
{
	if (tasks.size()==1)
		return tasks.begin()->second;

	std::string task = rii->getTaskForCompound(compound);
	return tasks.find(task);
}

void
TaskDeliverer::addTaskInfoInterface(lte::controlplane::TaskInfoInterface* _rii)
{
	// late dependency injection. At the point in time when this is called, it
	// is also safe to resolve the other dependencies, see below
	this->rii = _rii;

	// fill tasks registry
        wns::ldk::Link<wns::ldk::IDelivererReceptacle>::ExchangeContainer allFUs = this->get();
	for (wns::ldk::Link<wns::ldk::IDelivererReceptacle>::ExchangeContainer::const_iterator iter = allFUs.begin();
		 iter != allFUs.end();
		 ++iter)
	{
            assureType((*iter)->getFU(), lte::helper::HasModeName*);
            lte::helper::HasModeName* fu = dynamic_cast<lte::helper::HasModeName*>((*iter)->getFU());
            tasks.insert(fu->getTaskSuffix(), ((*iter)->getFU()));
	}
}

TaskDispatcherIncoming::TaskDispatcherIncoming(wns::ldk::fun::FUN* fun,
											   const wns::pyconfig::View& config) :
	wns::ldk::CommandTypeSpecifier<>(fun),
	wns::ldk::HasReceptor<wns::ldk::RoundRobinReceptor>(),
	wns::ldk::HasConnector<>(),
	wns::ldk::HasDeliverer<TaskDeliverer>(),
	wns::Cloneable<TaskDispatcherIncoming>(),
	wns::ldk::Forwarding<TaskDispatcherIncoming>(),
	lte::helper::HasModeName(config)
{
}

TaskDispatcherIncoming::~TaskDispatcherIncoming()
{}

void
TaskDispatcherIncoming::onFUNCreated()
{
	// Obtain a pointer to the service that provides mapping from compound to
	// task
	lte::controlplane::TaskInfoInterface* rii = getFUN()
		->findFriend<lte::controlplane::TaskInfoInterface*>(modeBase+separator+"phyUser");
	// Inject the dependency into our deliverer
	dynamic_cast<TaskDeliverer*>(this->getDeliverer())->addTaskInfoInterface(rii);
}
