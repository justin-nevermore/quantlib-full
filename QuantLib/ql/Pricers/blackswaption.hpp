
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file blackswaption.hpp
    \brief Swaption calculated using the Black formula
*/

#ifndef quantlib_pricers_black_swaption_h
#define quantlib_pricers_black_swaption_h

#include <ql/blackmodel.hpp>
#include <ql/Pricers/swaptionpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        //! Swaption priced by the Black formula
        class BlackSwaption : 
            public PricingEngines::GenericModelEngine<
                        BlackModel, 
                        Instruments::Swaption::arguments,
                        Instruments::Swaption::results> {
          public:
            BlackSwaption(const Handle<BlackModel>& mod)
            : PricingEngines::GenericModelEngine<
                        BlackModel, 
                        Instruments::Swaption::arguments,
                        Instruments::Swaption::results>(mod) {}
            void calculate() const;
        };

    }

}

#endif
