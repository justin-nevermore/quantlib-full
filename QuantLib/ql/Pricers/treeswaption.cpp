/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file treeswaption.cpp
    \brief European swaption calculated using finite differences

    \fullpath
    ql/Pricers/%treeswaption.cpp
*/

// $Id$

#include "ql/Pricers/treeswaption.hpp"
#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace Pricers {

        using namespace InterestRateModelling;
        using namespace Lattices;

        void TreeSwaption::calculate() const {

            Handle<Tree> tree;

            if (tree_.isNull()) {
                QL_REQUIRE(!model_.isNull(), "You must first define a model");

                std::list<Time> times(0);
                Size i;
                for (i=0; i<parameters_.exerciseTimes.size(); i++)
                    times.push_back(parameters_.exerciseTimes[i]);
                for (i=0; i<parameters_.fixedPayTimes.size(); i++)
                    times.push_back(parameters_.fixedPayTimes[i]);
                for (i=0; i<parameters_.floatingResetTimes.size(); i++)
                    times.push_back(parameters_.floatingResetTimes[i]);
                for (i=0; i<parameters_.floatingPayTimes.size(); i++)
                    times.push_back(parameters_.floatingPayTimes[i]);
                times.sort();
                times.unique();

                Handle<OneFactorModel> model(model_);
                QL_REQUIRE(!model.isNull(), "Only 1-d trees are supported");

                TimeGrid timeGrid(times, timeSteps_);
                tree = model->tree(timeGrid);
            } else {
                tree = tree_;
            }

            Handle<NumericalDerivative> swaption(
                new NumericalSwaption(tree, parameters_));


            tree->initialize(swaption, parameters_.exerciseTimes.back());
            tree->rollback(swaption, parameters_.exerciseTimes.front());

            results_.value = tree->presentValue(swaption);
/*
            if (!model_.isNull())
                std::cout << "Theoretical value: " << model_->termStructure()->discount(parameters_.floatingPayTimes[0])*100.0 << std::endl;
            std::cout << "Swap price: " << swap->values()[0] << std::endl;
*/
        }

    }

}
