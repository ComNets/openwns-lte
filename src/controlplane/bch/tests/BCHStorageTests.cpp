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

    #include <LTE/controlplane/bch/BCHStorage.hpp>

    #include <WNS/simulator/ISimulator.hpp>
    #include <WNS/events/scheduler/Interface.hpp>
    #include <WNS/CppUnit.hpp>
    #include <WNS/events/NoOp.hpp>
    #include <WNS/pyconfig/Parser.hpp>

    #include <boost/lambda/lambda.hpp>

    namespace lte { namespace controlplane { namespace bch { namespace tests {

        class BCHStorageTests :
            public wns::TestFixture
        {
            CPPUNIT_TEST_SUITE( BCHStorageTests );
            CPPUNIT_TEST( testGet );
            CPPUNIT_TEST( testBestSINR );
            CPPUNIT_TEST( testBestRxPwr );
            CPPUNIT_TEST( testBestDist );
            CPPUNIT_TEST( testBestPathloss );
            CPPUNIT_TEST( testUpdate );
            CPPUNIT_TEST( testReset );
            CPPUNIT_TEST( testEqualMeasurements );
            CPPUNIT_TEST( testOrderDoesntMatter );
            CPPUNIT_TEST( testGetBestInRange );
            CPPUNIT_TEST_SUITE_END();

            BCHStorage<wns::service::dll::UnicastAddress> testee;
            BCHRecordPtr m1;
            BCHRecordPtr m2;
            BCHRecordPtr m3;

            wns::service::dll::UnicastAddress k1;
            wns::service::dll::UnicastAddress k2;
            wns::service::dll::UnicastAddress k3;

            void
            prepare()
            {
                wns::simulator::getEventScheduler()->reset();
                testee.reset();
                // prepare 3 Addresses
                k1 = wns::service::dll::UnicastAddress(1);
                k2 = wns::service::dll::UnicastAddress(2);
                k3 = wns::service::dll::UnicastAddress(3);

                // prepare 3 Measurements
                m1 = BCHRecordPtr( new BCHRecord(k1,
                                wns::Ratio::from_dB(10),
                                wns::Ratio::from_dB(100),
                                wns::Power::from_dBm(-60),
                                200.0,
                                0) );
                m2 = BCHRecordPtr( new BCHRecord(k2,
                                wns::Ratio::from_dB( 5),
                                wns::Ratio::from_dB(102),
                                wns::Power::from_dBm(-40),
                                400.0,
                                0) );
                m3 = BCHRecordPtr( new BCHRecord(k3,
                                wns::Ratio::from_dB(15),
                                wns::Ratio::from_dB(98),
                                wns::Power::from_dBm(-70),
                                300.0,
                                0) );
                testee.store(k1, m1);
                testee.store(k2, m2);
                testee.store(k3, m3);
            }

            void
            cleanup()
            {
                m1 = BCHRecordPtr();
                m2 = BCHRecordPtr();
                m3 = BCHRecordPtr();
            }

        public:
            void
            testGet()
            {
                wns::service::dll::UnicastAddress search(2);

                CPPUNIT_ASSERT_DOUBLES_EQUAL( testee.get(search)->distance, 400.0 , 1E-9);
                CPPUNIT_ASSERT_EQUAL( testee.get(search)->sinr, wns::Ratio::from_dB(5));
                CPPUNIT_ASSERT_EQUAL( testee.get(search)->rxpower, wns::Power::from_dBm(-40));

                wns::service::dll::UnicastAddress searchUnknown(4);

                CPPUNIT_ASSERT( testee.get(searchUnknown) == BCHRecordPtr() );

            }

            void
            testBestSINR()
            {
                CPPUNIT_ASSERT( testee.getBest<compare::BestSINR>() == m3);
            }

            void
            testBestRxPwr()
            {
                CPPUNIT_ASSERT( testee.getBest<compare::BestRXPWR>() == m2);
            }

            void
            testBestDist()
            {
                CPPUNIT_ASSERT( testee.getBest<compare::BestDIST>() == m1);
            }

            void
            testBestPathloss()
            {
                CPPUNIT_ASSERT( testee.getBest<compare::BestPathloss>() == m3);
            }

            void
            testUpdate()
            {
                wns::service::dll::UnicastAddress someAddress(1);

                BCHRecordPtr update = BCHRecordPtr( new BCHRecord(someAddress,
                                        wns::Ratio::from_dB(16),
                                        wns::Ratio::from_dB(-96),
                                        wns::Power::from_dBm(-30),
                                        400.0, 0) );
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == m1);
                CPPUNIT_ASSERT(testee.getBest<compare::BestSINR>() == m3);
                CPPUNIT_ASSERT(testee.getBest<compare::BestRXPWR>() == m2);
                CPPUNIT_ASSERT(testee.getBest<compare::BestPathloss>() == m3);

                testee.store(someAddress, update);

                CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(3), testee.getAll().size() );

                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == m3);
                CPPUNIT_ASSERT(testee.getBest<compare::BestSINR>() == update);
                CPPUNIT_ASSERT(testee.getBest<compare::BestRXPWR>() == update);
                CPPUNIT_ASSERT(testee.getBest<compare::BestPathloss>() == update);
            }

            void
            testReset()
            {
                CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(3), testee.getAll().size() );
                testee.reset();
                CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(0), testee.getAll().size() );
            }

            void
            testEqualMeasurements()
            {
                BCHRecordPtr first = BCHRecordPtr( new BCHRecord(wns::service::dll::UnicastAddress(1),
                                        wns::Ratio::from_dB(16),
                                        wns::Ratio::from_dB(-100),
                                        wns::Power::from_dBm(-30),
                                        400.0, 0) );

                // make some time pass
                wns::simulator::getEventScheduler()->schedule(wns::events::NoOp(), 42.0);
                wns::simulator::getEventScheduler()->processOneEvent();

                BCHRecordPtr second = BCHRecordPtr( new BCHRecord(wns::service::dll::UnicastAddress(2),
                                        wns::Ratio::from_dB(16),
                                        wns::Ratio::from_dB(-100),
                                        wns::Power::from_dBm(-30),
                                        400.0, 0) );

                CPPUNIT_ASSERT_EQUAL(  simTimeType(0.0),  first->timeStamp );
                CPPUNIT_ASSERT_EQUAL( simTimeType(42.0), second->timeStamp );

                // first store them in the correct order
                testee.reset();
                testee.store(k1, first);
                testee.store(k2, second);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() != first);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == second);

                // now store them in reverse order, shouldn't matter
                testee.reset();
                testee.store(k2, second);
                testee.store(k1, first);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() != first);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == second);
            }

            void
            testOrderDoesntMatter()
            {
                testee.reset();
                testee.store(k1, m1); testee.store(k3, m3); testee.store(k2, m2);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == m1);
                CPPUNIT_ASSERT(testee.getBest<compare::BestSINR>() == m3);
                CPPUNIT_ASSERT(testee.getBest<compare::BestRXPWR>() == m2);
                CPPUNIT_ASSERT(testee.getBest<compare::BestPathloss>() == m3);
                testee.reset();
                testee.store(k2, m2); testee.store(k1, m1); testee.store(k3, m3);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == m1);
                CPPUNIT_ASSERT(testee.getBest<compare::BestSINR>() == m3);
                CPPUNIT_ASSERT(testee.getBest<compare::BestRXPWR>() == m2);
                CPPUNIT_ASSERT(testee.getBest<compare::BestPathloss>() == m3);
                testee.reset();
                testee.store(k2, m2); testee.store(k3, m3); testee.store(k1, m1);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == m1);
                CPPUNIT_ASSERT(testee.getBest<compare::BestSINR>() == m3);
                CPPUNIT_ASSERT(testee.getBest<compare::BestRXPWR>() == m2);
                CPPUNIT_ASSERT(testee.getBest<compare::BestPathloss>() == m3);
                testee.reset();
                testee.store(k3, m3); testee.store(k1, m1); testee.store(k2, m2);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == m1);
                CPPUNIT_ASSERT(testee.getBest<compare::BestSINR>() == m3);
                CPPUNIT_ASSERT(testee.getBest<compare::BestRXPWR>() == m2);
                CPPUNIT_ASSERT(testee.getBest<compare::BestPathloss>() == m3);
                testee.reset();
                testee.store(k3, m3); testee.store(k2, m2); testee.store(k1, m1);
                CPPUNIT_ASSERT(testee.getBest<compare::BestDIST>() == m1);
                CPPUNIT_ASSERT(testee.getBest<compare::BestSINR>() == m3);
                CPPUNIT_ASSERT(testee.getBest<compare::BestRXPWR>() == m2);
                CPPUNIT_ASSERT(testee.getBest<compare::BestPathloss>() == m3);

            }

            void
            testGetBestInRange()
            {
                BCHRecordPtr first = BCHRecordPtr( new BCHRecord(wns::service::dll::UnicastAddress(2),
                                                                wns::Ratio::from_dB(16),
                                                                wns::Ratio::from_dB(-100),
                                                                wns::Power::from_dBm(-30),
                                                                200.0, 0) );
                BCHRecordPtr second = BCHRecordPtr( new BCHRecord(wns::service::dll::UnicastAddress(2),
                                                                wns::Ratio::from_dB(2),
                                                                wns::Ratio::from_dB(-150),
                                                                wns::Power::from_dBm(-22),
                                                                100.0, 0) );
                BCHRecordPtr third = BCHRecordPtr( new BCHRecord(wns::service::dll::UnicastAddress(2),
                                                                wns::Ratio::from_dB(18),
                                                                wns::Ratio::from_dB(-60),
                                                                wns::Power::from_dBm(-16),
                                                                250.0, 0) );

                testee.store(k1, first);
                testee.store(k2, second);
                testee.store(k3, third);

                using namespace boost::lambda;
                boost::function<bool (wns::Ratio, wns::Ratio)> cmpr = _1 <= _2;
                boost::function<bool (wns::Power, wns::Power)> cmpp = _1 <= _2;
                boost::function<bool (double, double)> cmpd = _1 <= _2;

                wns::Ratio lowerBoundpl = wns::Ratio::from_dB(-150);
                wns::Ratio upperBoundpl = wns::Ratio::from_dB(-90);

                boost::function<wns::Ratio (BCHRecord*)> getterpl = (_1 ->* &BCHRecord::pathloss);

                std::vector<BCHRecordPtr> r = testee.getBestInRange(lowerBoundpl, upperBoundpl, getterpl, cmpr);

                CPPUNIT_ASSERT_EQUAL( (size_t) 2, r.size());

                CPPUNIT_ASSERT( (r[0]->pathloss == wns::Ratio::from_dB(-150) && r[1]->pathloss == wns::Ratio::from_dB(-100)) ||
                                (r[1]->pathloss == wns::Ratio::from_dB(-150) && r[0]->pathloss == wns::Ratio::from_dB(-100)));

                wns::Ratio lowerBoundsinr = wns::Ratio::from_dB(17);
                wns::Ratio upperBoundsinr = wns::Ratio::from_dB(19);

                boost::function<wns::Ratio (BCHRecord*)> gettersinr = (_1 ->* &BCHRecord::sinr);

                r = testee.getBestInRange(lowerBoundsinr, upperBoundsinr, gettersinr, cmpr);

                CPPUNIT_ASSERT_EQUAL( (size_t) 1, r.size());

                CPPUNIT_ASSERT(r[0]->sinr == wns::Ratio::from_dB(18));

                wns::Power lowerBoundrxpwr = wns::Power::from_dBm(-40);
                wns::Power upperBoundrxpwr = wns::Power::from_dBm(-20);

                boost::function<wns::Power (BCHRecord*)> getterrxpwr = (_1 ->* &BCHRecord::rxpower);

                r = testee.getBestInRange(lowerBoundrxpwr, upperBoundrxpwr, getterrxpwr, cmpp);

                CPPUNIT_ASSERT_EQUAL( (size_t) 2, r.size());

                CPPUNIT_ASSERT( (r[0]->rxpower == wns::Power::from_dBm(-30) && r[1]->rxpower == wns::Power::from_dBm(-22)) ||
                                (r[1]->rxpower == wns::Power::from_dBm(-30) && r[0]->rxpower == wns::Power::from_dBm(-22)));

                double lowerBounddist = 50;
                double upperBounddist = 150;

                boost::function<double (BCHRecord*)> getterdist = (_1 ->* &BCHRecord::distance);

                r = testee.getBestInRange(lowerBounddist, upperBounddist, getterdist, cmpd);

                CPPUNIT_ASSERT_EQUAL( (size_t) 1, r.size());

                CPPUNIT_ASSERT(r[0]->distance == 100);
            }

        };


        CPPUNIT_TEST_SUITE_REGISTRATION( BCHStorageTests );

    } // tests
    } // bch
    } // controlplane
    } // lte



