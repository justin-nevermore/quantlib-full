/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco

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

/*! \file couponpricer.hpp
    \brief Coupon pricers
*/

#ifndef quantlib_coupon_pricer_hpp
#define quantlib_coupon_pricer_hpp

#include <ql/termstructures/capvolstructures.hpp>
#include <ql/termstructures/swaptionvolstructure.hpp>
#include <ql/cashflow.hpp>
#include <ql/option.hpp>

namespace QuantLib {

    class FloatingRateCoupon;
    class IborCoupon;

    //! generic pricer for floating-rate coupons
    class FloatingRateCouponPricer: public virtual Observer,
                                    public virtual Observable {
      public:
        virtual ~FloatingRateCouponPricer() {}
        //! \name required interface
        //@{
        virtual Real swapletPrice() const = 0;
        virtual Rate swapletRate() const = 0;
        virtual Real capletPrice(Rate effectiveCap) const = 0;
        virtual Rate capletRate(Rate effectiveCap) const = 0;
        virtual Real floorletPrice(Rate effectiveFloor) const = 0;
        virtual Rate floorletRate(Rate effectiveFloor) const = 0;
        virtual void initialize(const FloatingRateCoupon& coupon) = 0;
        //@}
        //! \name Observer interface
        //@{
        void update(){notifyObservers();}
        //@}
    };

    //! base pricer for capped/floored Ibor coupons
    class IborCouponPricer : public FloatingRateCouponPricer {
      public:
        IborCouponPricer(const Handle<CapletVolatilityStructure>& capletVol)
        : capletVol_(capletVol) { registerWith(capletVol_); }

        Handle<CapletVolatilityStructure> capletVolatility() const{
            return capletVol_;
        }
        void setCapletVolatility(
                         const Handle<CapletVolatilityStructure>& capletVol) {
            unregisterWith(capletVol_);
            capletVol_ = capletVol;
            QL_REQUIRE(!capletVol_.empty(), "no adequate capletVol given");
            registerWith(capletVol_);
            update();
        }
      private:
        Handle<CapletVolatilityStructure> capletVol_;
    };

    //! Black-formula pricer for capped/floored Ibor coupons
    class BlackIborCouponPricer : public IborCouponPricer {
      public:
        BlackIborCouponPricer(
                          const Handle<CapletVolatilityStructure>& capletVol =
                                          Handle<CapletVolatilityStructure>())
        : IborCouponPricer(capletVol) {};
        virtual void initialize(const FloatingRateCoupon& coupon);
        /* */
        Real swapletPrice() const;
        Rate swapletRate() const;
        Real capletPrice(Rate effectiveCap) const;
        Rate capletRate(Rate effectiveCap) const;
        Real floorletPrice(Rate effectiveFloor) const;
        Rate floorletRate(Rate effectiveFloor) const;

      protected:
        Real optionletPrice(Option::Type optionType,
                            Real effStrike) const;

      private:
        Rate adjustedFixing() const;

        const IborCoupon* coupon_;
        Real discount_;
        Real gearing_;
        Spread spread_;
        Real spreadLegValue_;
    };

    //! base pricer for vanilla CMS coupons
    class CmsCouponPricer : public FloatingRateCouponPricer {
      public:
        CmsCouponPricer(const Handle<SwaptionVolatilityStructure>& swaptionVol)
        : swaptionVol_(swaptionVol) { registerWith(swaptionVol_); }

        Handle<SwaptionVolatilityStructure> swaptionVolatility() const{
            return swaptionVol_;
        }
        void setSwaptionVolatility(
                     const Handle<SwaptionVolatilityStructure>& swaptionVol) {
            unregisterWith(swaptionVol_);
            swaptionVol_ = swaptionVol;
            QL_REQUIRE(!swaptionVol_.empty(), "no adequate swaptionVol given");
            registerWith(swaptionVol_);
            update();
        }
      private:
        Handle<SwaptionVolatilityStructure> swaptionVol_;
    };

    void setCouponPricer(const Leg& leg,
                         const boost::shared_ptr<FloatingRateCouponPricer>&);

    void setCouponPricers(
            const Leg& leg,
            const std::vector<boost::shared_ptr<FloatingRateCouponPricer> >&);

}


#endif
