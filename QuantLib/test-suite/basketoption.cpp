
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2003 RiskMap srl

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

#include "basketoption.hpp"
#include "utilities.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/basketoption.hpp>
#include <ql/PricingEngines/Basket/stulzengine.hpp>
#include <ql/PricingEngines/Basket/mcbasketengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    std::string basketTypeToString(BasketOption::BasketType basketType) {

        switch (basketType) {
        case BasketOption::Min:
            return "Basket::Min";
        case BasketOption::Max:
            return "Basket::Max";
        }

        QL_FAIL("basketTypeToString : unknown basket option type");
    }

    void basketOptionTestFailed(std::string greekName,
                                BasketOption::BasketType basketType,
                                const Handle<StrikedTypePayoff>& payoff,
                                const Handle<Exercise>& exercise,
                                double s1,
                                double s2,
                                double q1,
                                double q2,
                                double r,
                                Date today,
                                DayCounter dc,
                                double v1,
                                double v2,
                                double rho,
                                double expected,
                                double calculated,
                                double error,
                                double tolerance) {

        Time t = dc.yearFraction(today, exercise->lastDate());

        CPPUNIT_FAIL(exerciseTypeToString(exercise) + " "
            + OptionTypeFormatter::toString(payoff->optionType()) +
            " option on "
            + basketTypeToString(basketType) +
            " with "
            + payoffTypeToString(payoff) + " payoff:\n"
            "1st underlying value: "
            + DoubleFormatter::toString(s1) + "\n"
            "2nd underlying value: "
            + DoubleFormatter::toString(s2) + "\n"
            "              strike: "
            + DoubleFormatter::toString(payoff->strike()) +"\n"
            "  1st dividend yield: "
            + DoubleFormatter::toString(q1) + "\n"
            "  2nd dividend yield: "
            + DoubleFormatter::toString(q2) + "\n"
            "      risk-free rate: "
            + DoubleFormatter::toString(r) + "\n"
            "      reference date: "
            + DateFormatter::toString(today) + "\n"
            "            maturity: "
            + DateFormatter::toString(exercise->lastDate()) + "\n"
            "      time to expiry: "
            + DoubleFormatter::toString(t) + "\n"
            "1st asset volatility: "
            + DoubleFormatter::toString(v1) + "\n"
            "2nd asset volatility: "
            + DoubleFormatter::toString(v2) + "\n"
            "         correlation: "
            + DoubleFormatter::toString(rho) + "\n\n"
            "    expected   " + greekName + ": "
            + DoubleFormatter::toString(expected) + "\n"
            "    calculated " + greekName + ": "
            + DoubleFormatter::toString(calculated) + "\n"
            "    error:            "
            + DoubleFormatter::toString(error) + "\n"
            "    tolerance:        "
            + DoubleFormatter::toString(tolerance));
    }

    void basketOptionThreeTestFailed(std::string greekName,
                                BasketOption::BasketType basketType,
                                const Handle<StrikedTypePayoff>& payoff,
                                const Handle<Exercise>& exercise,
                                double s1,
                                double s2,
                                double s3,
                                double r,
                                Date today,
                                DayCounter dc,
                                double v1,
                                double v2,
                                double v3,
                                double rho,
                                double expected,
                                double calculated,
                                double error,
                                double tolerance) {

        Time t = dc.yearFraction(today, exercise->lastDate());

        CPPUNIT_FAIL(exerciseTypeToString(exercise) + " "
            + OptionTypeFormatter::toString(payoff->optionType()) +
            " option on "
            + basketTypeToString(basketType) +
            " with "
            + payoffTypeToString(payoff) + " payoff:\n"
            "1st underlying value: "
            + DoubleFormatter::toString(s1) + "\n"
            "2nd underlying value: "
            + DoubleFormatter::toString(s2) + "\n"
            "3rd underlying value: "
            + DoubleFormatter::toString(s3) + "\n"
            "              strike: "
            + DoubleFormatter::toString(payoff->strike()) +"\n"
            "      risk-free rate: "
            + DoubleFormatter::toString(r) + "\n"
            "      reference date: "
            + DateFormatter::toString(today) + "\n"
            "            maturity: "
            + DateFormatter::toString(exercise->lastDate()) + "\n"
            "      time to expiry: "
            + DoubleFormatter::toString(t) + "\n"
            "1st asset volatility: "
            + DoubleFormatter::toString(v1) + "\n"
            "2nd asset volatility: "
            + DoubleFormatter::toString(v2) + "\n"
            "3rd asset volatility: "
            + DoubleFormatter::toString(v3) + "\n"
            "         correlation: "
            + DoubleFormatter::toString(rho) + "\n\n"
            "    expected   " + greekName + ": "
            + DoubleFormatter::toString(expected) + "\n"
            "    calculated " + greekName + ": "
            + DoubleFormatter::toString(calculated) + "\n"
            "    error:            "
            + DoubleFormatter::toString(error) + "\n"
            "    tolerance:        "
            + DoubleFormatter::toString(tolerance));
    }

    struct BasketOptionData {
        BasketOption::BasketType basketType;
        Option::Type type;
        double strike;
        double s1;
        double s2;
        double q1;
        double q2;
        double r;
        Time t; // years
        double v1;
        double v2;
        double rho;
        double result;
        double tol;
    };

    struct BasketOptionBarraquandThreeData {
        BasketOption::BasketType basketType;
        Option::Type type;
        double strike;
        double s1;
        double s2;
        double s3;
        double r;
        Time t; // months
        double v1;
        double v2;
        double v3;
        double rho;
        double result;
    };


}

void BasketOptionTest::testValues() {

    /*
        Data from:
        Excel spreadsheet www.maths.ox.ac.uk/~firth/computing/excel.shtml
        and
        "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 56-58
    */
    BasketOptionData values[] = {
        //      basketType,   optionType, strike,    s1,    s2,   q1,   q2,    r,    t,   v1,   v2,  rho, result, tol
        // data from http://www.maths.ox.ac.uk/~firth/computing/excel.shtml
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 10.898, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70,  8.483, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50,  6.844, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  5.531, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10,  4.413, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.70, 0.00,  4.981, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.30, 0.00,  4.159, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.10, 0.00,  2.597, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.10, 0.50,  4.030, 1.0e-3},

        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 17.565, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70, 19.980, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50, 21.619, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 22.932, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10, 24.049, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0,  80.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 16.508, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0,  80.0,  80.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  8.049, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0,  80.0, 120.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 30.141, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 120.0, 120.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 42.889, 1.0e-3},

        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 11.369, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70, 12.856, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50, 13.890, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 14.741, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10, 15.485, 1.0e-3},

        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 0.50, 0.30, 0.30, 0.10, 11.893, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 0.25, 0.30, 0.30, 0.10,  8.881, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 2.00, 0.30, 0.30, 0.10, 19.268, 1.0e-3},

        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90,  7.339, 1.0e-3},
        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70,  5.853, 1.0e-3},
        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50,  4.818, 1.0e-3},
        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  3.967, 1.0e-3},
        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10,  3.223, 1.0e-3},

        //      basketType,   optionType, strike,    s1,    s2,   q1,   q2,    r,    t,   v1,   v2,  rho,  result, tol
        // data from "Option pricing formulas" VB code + spreadsheet 
        {BasketOption::Min, Option::Call,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  4.8177, 1.0e-4},
        {BasketOption::Max, Option::Call,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63, 11.6323, 1.0e-4},
        {BasketOption::Min,  Option::Put,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  2.0376, 1.0e-4},
        {BasketOption::Max,  Option::Put,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  0.5731, 1.0e-4},
        {BasketOption::Min, Option::Call,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  2.9340, 1.0e-4},
        {BasketOption::Min,  Option::Put,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  3.5224, 1.0e-4},
        // data from "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 58
        {BasketOption::Max, Option::Call,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  8.0701, 1.0e-4},
        {BasketOption::Max,  Option::Put,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  1.2181, 1.0e-4}
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot1(new SimpleQuote(0.0));
    Handle<SimpleQuote> spot2(new SimpleQuote(0.0));

    Handle<SimpleQuote> qRate1(new SimpleQuote(0.0));
    Handle<TermStructure> qTS1 = makeFlatCurve(qRate1, dc);
    Handle<SimpleQuote> qRate2(new SimpleQuote(0.0));
    Handle<TermStructure> qTS2 = makeFlatCurve(qRate2, dc);

    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);

    Handle<SimpleQuote> vol1(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS1 = makeFlatVolatility(vol1, dc);
    Handle<SimpleQuote> vol2(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS2 = makeFlatVolatility(vol2, dc);


    Handle<PricingEngine> engine(new StulzEngine);

    double mcRelativeErrorTolerance = 0.01;
    Handle<PricingEngine> mcEngine(new MCBasketEngine<PseudoRandom, Statistics> 
        (1, false, false, Null<int>(), 0.005, Null<int>(), 42));

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<PlainVanillaPayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> exercise(new EuropeanExercise(exDate));

        spot1 ->setValue(values[i].s1);
        spot2 ->setValue(values[i].s2);
        qRate1->setValue(values[i].q1);
        qRate2->setValue(values[i].q2);
        rRate ->setValue(values[i].r );
        vol1  ->setValue(values[i].v1);
        vol2  ->setValue(values[i].v2);

        Handle<BlackScholesStochasticProcess> stochProcess1(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot1),
                RelinkableHandle<TermStructure>(qTS1),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS1)));

        Handle<BlackScholesStochasticProcess> stochProcess2(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot2),
                RelinkableHandle<TermStructure>(qTS2),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS2)));

        std::vector<Handle<BlackScholesStochasticProcess> > procs =
            std::vector<Handle<BlackScholesStochasticProcess> >();
        procs.push_back(stochProcess1);
        procs.push_back(stochProcess2);

        Matrix correlationMatrix(2,2, values[i].rho);
        for (int j=0; j < 2; j++) {
            correlationMatrix[j][j] = 1.0;
        }

        BasketOption basketOption(values[i].basketType, procs, payoff, 
                                  exercise, correlationMatrix, engine);

        // analytic engine
        double calculated = basketOption.NPV();
        double expected = values[i].result;
        double error = QL_FABS(calculated-expected);
        if (error > values[i].tol) {
            basketOptionTestFailed("value",
                values[i].basketType, payoff, exercise, values[i].s1, values[i].s2,
                values[i].q1, values[i].q2, values[i].r,
                today, dc, values[i].v1, values[i].v2, values[i].rho,
                values[i].result, calculated, error, values[i].tol);
        }

        // mc engine
        basketOption.setPricingEngine(mcEngine);
        calculated = basketOption.NPV();
        double relError = relativeError(calculated, expected, values[i].s1);
        if (relError > mcRelativeErrorTolerance ) {
            basketOptionTestFailed("MC value",
                values[i].basketType, payoff, exercise, values[i].s1, values[i].s2,
                values[i].q1, values[i].q2, values[i].r,
                today, dc, values[i].v1, values[i].v2, values[i].rho,
                values[i].result, calculated, relError, mcRelativeErrorTolerance);
        }

    }
}


void BasketOptionTest::testBarraquandThreeValues() {

    /*
        Data from:
        "Numerical Valuation of High Dimensional American Securities"
        Barraquand, J. and Martineau, D.
        Journal of Financial and Quantitative Analysis 1995 3(30) 383-405        
    */
    BasketOptionBarraquandThreeData  values[] = {
        // time in months is with 30 days to the month..
        // basketType, optionType,       strike,    s1,    s2,   s3,    r,    t,   v1,   v2,  v3,  rho, result,         
        // Table 2
        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 8.59},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 3.84},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.89},
        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 12.55},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 7.87},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 4.26},
        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 15.29},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 10.72},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 6.96},

        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 7.78},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 3.18},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.82},
        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 10.97},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 6.69},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 3.70},
        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 13.23},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 9.11},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 5.98},

        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 6.53},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 2.38},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.74},
        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 8.51},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 4.92},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 2.97},
        {BasketOption::Max, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 10.04},
        {BasketOption::Max, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 6.64},
        {BasketOption::Max, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 4.61}/*,

        // Table 3
        // not working yet...
        //{BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.00},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.13},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 2.26},
        {BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 0.01},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 0.25},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 1.55},
        {BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 0.03},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 0.31},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 1.41},        
       
        {BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.00},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.38},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 3.00},
        {BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 0.07},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 0.72},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 2.65},
        {BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 0.17},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 0.91},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 2.63},        
       
        {BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.01},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.84},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 4.18},
        {BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 0.19},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 1.51},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 4.49},
        {BasketOption::Max, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 0.41},
        {BasketOption::Max, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 1.87},
        {BasketOption::Max, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 4.70} 
        */
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot1(new SimpleQuote(0.0));
    Handle<SimpleQuote> spot2(new SimpleQuote(0.0));
    Handle<SimpleQuote> spot3(new SimpleQuote(0.0));

    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);

    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);

    Handle<SimpleQuote> vol1(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS1 = makeFlatVolatility(vol1, dc);
    Handle<SimpleQuote> vol2(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS2 = makeFlatVolatility(vol2, dc);
    Handle<SimpleQuote> vol3(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS3 = makeFlatVolatility(vol3, dc);

    double mcRelativeErrorTolerance = 0.01;
    Handle<PricingEngine> mcEngine(new MCBasketEngine<PseudoRandom, Statistics>
        (1, false, false, Null<int>(), 0.005, Null<int>(), 42));

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<PlainVanillaPayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*30+0.5));
        Handle<Exercise> exercise(new EuropeanExercise(exDate));

        spot1 ->setValue(values[i].s1);
        spot2 ->setValue(values[i].s2);
        spot3 ->setValue(values[i].s3);
        rRate ->setValue(values[i].r );
        vol1  ->setValue(values[i].v1);
        vol2  ->setValue(values[i].v2);
        vol3  ->setValue(values[i].v3);

        Handle<BlackScholesStochasticProcess> stochProcess1(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot1),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS1)));

        Handle<BlackScholesStochasticProcess> stochProcess2(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot2),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS2)));

        Handle<BlackScholesStochasticProcess> stochProcess3(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot3),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS3)));

        std::vector<Handle<BlackScholesStochasticProcess> > procs =
            std::vector<Handle<BlackScholesStochasticProcess> >();
        procs.push_back(stochProcess1);
        procs.push_back(stochProcess2);
        procs.push_back(stochProcess3);

        Matrix correlation(3,3, values[i].rho);
        for (int j=0; j < 3; j++) {
            correlation[j][j] = 1.0;
        }

        BasketOption basketOption(values[i].basketType, procs, payoff, 
                                  exercise, correlation, mcEngine);

        // mc engine
        double calculated = basketOption.NPV();
        double expected = values[i].result;
        double relError = relativeError(calculated, expected, values[i].s1);
        if (relError > mcRelativeErrorTolerance ) {
            basketOptionThreeTestFailed("MC value",
                values[i].basketType, payoff, exercise, 
                values[i].s1, values[i].s2, values[i].s3, values[i].r,
                today, dc, values[i].v1, values[i].v2, values[i].v3, values[i].rho,
                values[i].result, calculated, relError, mcRelativeErrorTolerance);
        }

    }
}


CppUnit::Test* BasketOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Basket option tests");
    tests->addTest(new CppUnit::TestCaller<BasketOptionTest>
                   ("Testing basket options against correct values",
                    &BasketOptionTest::testValues));
    tests->addTest(new CppUnit::TestCaller<BasketOptionTest>
                   ("Testing basket options against Barraquand values",
                    &BasketOptionTest::testBarraquandThreeValues));

    return tests;
}

