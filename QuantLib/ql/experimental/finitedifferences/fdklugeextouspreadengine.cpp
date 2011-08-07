/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#include <ql/exercise.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/processes/klugeextouprocess.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/fdmamericanstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmbermudanstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/exponentialjump1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdmklugeextousolver.hpp>
#include <ql/experimental/finitedifferences/fdmsimpleprocess1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdklugeextouspreadengine.hpp>
#include <ql/experimental/finitedifferences/fdmspreadpayoffinnervalue.hpp>

namespace QuantLib {

    FdKlugeExtOUSpreadEngine::FdKlugeExtOUSpreadEngine(
        const boost::shared_ptr<KlugeExtOUProcess>& klugeOUProcess,
        const boost::shared_ptr<YieldTermStructure>& rTS,
        Size tGrid, Size xGrid, Size yGrid, Size uGrid,
        const boost::shared_ptr<GasShape>& gasShape,
        const boost::shared_ptr<PowerShape>& powerShape,
        const FdmSchemeDesc& schemeDesc)
    : klugeOUProcess_(klugeOUProcess),
      rTS_  (rTS),
      tGrid_(tGrid),
      xGrid_(xGrid),
      yGrid_(yGrid),
      uGrid_(uGrid),
      gasShape_(gasShape),
      powerShape_(powerShape),
      schemeDesc_(schemeDesc) {
    }

    void FdKlugeExtOUSpreadEngine::calculate() const {
        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        dim.push_back(yGrid_);
        dim.push_back(uGrid_);
        const boost::shared_ptr<FdmLinearOpLayout> layout(
                                            new FdmLinearOpLayout(dim));

        // 2. Mesher
        const Time maturity
            = rTS_->dayCounter().yearFraction(rTS_->referenceDate(),
                                              arguments_.exercise->lastDate());
        const boost::shared_ptr<ExtOUWithJumpsProcess> klugeProcess
                                          = klugeOUProcess_->getKlugeProcess();
        const boost::shared_ptr<StochasticProcess1D> ouProcess
                        = klugeProcess->getExtendedOrnsteinUhlenbeckProcess();
        const boost::shared_ptr<Fdm1dMesher> xMesher(
            new FdmSimpleProcess1dMesher(xGrid_, ouProcess,maturity));

        const boost::shared_ptr<Fdm1dMesher> yMesher(
            new ExponentialJump1dMesher(yGrid_,
                                        klugeProcess->beta(),
                                        klugeProcess->jumpIntensity(),
                                        klugeProcess->eta()));

        const boost::shared_ptr<Fdm1dMesher> uMesher(
            new FdmSimpleProcess1dMesher(uGrid_,
                                         klugeOUProcess_->getExtOUProcess(),
                                         maturity));

        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(xMesher);
        meshers.push_back(yMesher);
        meshers.push_back(uMesher);
        const boost::shared_ptr<FdmMesher> mesher (
                                   new FdmMesherComposite(layout, meshers));

        // 3. Calculator
        boost::shared_ptr<BasketPayoff> basketPayoff =
            boost::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff);
        QL_REQUIRE(basketPayoff," basket payoff expected");

        const boost::shared_ptr<Payoff> zeroStrikeCall(
            new PlainVanillaPayoff(Option::Call, 0.0));

        const boost::shared_ptr<FdmInnerValueCalculator> gasPrice(
            new FdmExpExtOUInnerValueCalculator(zeroStrikeCall,
                                                mesher, gasShape_, 2));

        const boost::shared_ptr<FdmInnerValueCalculator> powerPrice(
            new FdmExtOUJumpModelInnerValue(zeroStrikeCall,mesher,powerShape_));

        const boost::shared_ptr<FdmInnerValueCalculator> calculator(
            new FdmSpreadPayoffInnerValue(basketPayoff, powerPrice, gasPrice));

        // 4. Step conditions
        const boost::shared_ptr<FdmStepConditionComposite> conditions =
            FdmStepConditionComposite::vanillaComposite(
                                DividendSchedule(), arguments_.exercise,
                                mesher, calculator,
                                rTS_->referenceDate(), rTS_->dayCounter());

        // 5. Boundary conditions
        const std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;

        // 6. set-up solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     calculator, maturity, tGrid_, 0 };

        const boost::shared_ptr<FdmKlugeExtOUSolver<3> > solver(
            new FdmKlugeExtOUSolver<3>(
                Handle<KlugeExtOUProcess>(klugeOUProcess_),
                rTS_, solverDesc, schemeDesc_));

        std::vector<Real> x(3);
        x[0] = klugeOUProcess_->initialValues()[0];
        x[1] = klugeOUProcess_->initialValues()[1];
        x[2] = klugeOUProcess_->initialValues()[2];

        results_.value = solver->valueAt(x);
    }
}
