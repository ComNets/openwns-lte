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

#ifndef LTE_CONTROLPLANE_TASKDISPATCHER_HPP
#define LTE_CONTROLPLANE_TASKDISPATCHER_HPP

#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/RoundRobinReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Forwarding.hpp>
#include <WNS/ldk/IDelivererReceptacle.hpp>

#include <WNS/ldk/multiplexer/Dispatcher.hpp>

#include <WNS/ldk/MultiLink.hpp>
#include <WNS/container/Registry.hpp>

#include <LTE/helper/HasModeName.hpp>

#include <WNS/Cloneable.hpp>

namespace lte { namespace controlplane {

	class NamedDispatcher :
		public wns::ldk::multiplexer::Dispatcher,
		public lte::helper::HasModeName,
		public wns::Cloneable<NamedDispatcher>
	{
	public:
		NamedDispatcher(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
			wns::ldk::multiplexer::Dispatcher(fun, config),
			lte::helper::HasModeName(config)
		{}

		virtual
		~NamedDispatcher(){}

		virtual wns::CloneableInterface*
		clone() const
		{
			return new NamedDispatcher(*this);
		}
	};


	class TaskInfoInterface
	{
	public:
		virtual
		~TaskInfoInterface(){};

		virtual std::string
		getTaskForCompound(const wns::ldk::CompoundPtr& compound) const = 0;
	};

	class TaskDeliverer :
		public wns::ldk::Deliverer,
                public wns::ldk::MultiLink<wns::ldk::IDelivererReceptacle>
	{
		wns::container::Registry<std::string, wns::ldk::IDelivererReceptacle*> tasks;
		lte::controlplane::TaskInfoInterface* rii;
	public:
		virtual
		~TaskDeliverer()
		{}

		virtual wns::ldk::IDelivererReceptacle*
		getAcceptor(const wns::ldk::CompoundPtr& compound);

		void
		addTaskInfoInterface(lte::controlplane::TaskInfoInterface* _rii);
	};

	class TaskDispatcherIncoming :
		public virtual wns::ldk::FunctionalUnit,
		public wns::ldk::CommandTypeSpecifier<>,
		public wns::ldk::HasReceptor<wns::ldk::RoundRobinReceptor>,
		public wns::ldk::HasConnector<>,
		public wns::ldk::HasDeliverer<TaskDeliverer>,
		public wns::Cloneable<TaskDispatcherIncoming>,
		public wns::ldk::Forwarding<TaskDispatcherIncoming>,
		public lte::helper::HasModeName
	{
	public:
		TaskDispatcherIncoming(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
		virtual
		~TaskDispatcherIncoming();

		// dependency resolution
		virtual void
		onFUNCreated();
	};


}}

#endif // not defined LTE_CONTROLPLANE_TASKDISPATCHER_HPP



