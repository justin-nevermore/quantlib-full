
/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmultiperiodengine.hpp
    \brief base engine for options with events happening at specific times
*/

#ifndef quantlib_fd_multi_period_engine_hpp
#define quantlib_fd_multi_period_engine_hpp

#include <ql/Instruments/dividendvanillaoption.hpp>
#include <ql/PricingEngines/Vanilla/fdvanillaengine.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>

namespace QuantLib {

    class FDMultiPeriodEngine : public DividendVanillaOption::engine,
                                public FDVanillaEngine {
      protected:
        FDMultiPeriodEngine(Size gridPoints=100, Size timeSteps=100,
                            bool timeDependent = false);
        Size timeStepPerPeriod_;
        mutable Array prices_;
        void calculate() const;
        mutable boost::shared_ptr<StandardStepCondition > stepCondition_;
        mutable boost::shared_ptr<StandardFiniteDifferenceModel> model_;
        virtual void executeIntermediateStep(Size step) const = 0;
        virtual void initializeStepCondition() const;
        virtual void initializeModel() const;
        Time getDividendTime(int i) const {
            return getYearFraction(arguments_.dividendDates[i]);
        }
    };

}


#endif
