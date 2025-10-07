/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include "ModelFixture.hpp"

#include "../HeatPumpAirToWaterHeating.hpp"
#include "../HeatPumpAirToWaterHeating_Impl.hpp"

#include "../HeatPumpAirToWaterHeatingSpeedData.hpp"
#include "../HeatPumpAirToWaterHeatingSpeedData_Impl.hpp"

#include "../Model.hpp"
#include "../Schedule.hpp"
#include "../ScheduleConstant.hpp"
#include "../Curve.hpp"
#include "../Curve_Impl.hpp"
#include "../CurveCubic.hpp"
#include "../CurveBicubic.hpp"

#include "../AirLoopHVAC.hpp"
#include "../PlantLoop.hpp"
#include "../Node.hpp"
#include "../Node_Impl.hpp"

#include "../ModelObjectList.hpp"
#include "../ModelObjectList_Impl.hpp"

using namespace openstudio;
using namespace openstudio::model;

TEST_F(ModelFixture, HeatPumpAirToWaterHeating_GettersSetters) {
  Model m;
  HeatPumpAirToWaterHeating awhp(m);

  awhp.setName("My HeatPumpAirToWaterHeating");

  // Availability Schedule Name: Required Object
  // Ctor default: Always On
  EXPECT_EQ(m.alwaysOnDiscreteSchedule(), awhp.availabilitySchedule());
  ScheduleConstant availabilitySchedule(m);
  EXPECT_TRUE(awhp.setAvailabilitySchedule(availabilitySchedule));
  EXPECT_EQ(availabilitySchedule, awhp.availabilitySchedule());

  // Rated Inlet Air Temperature: Required Double
  // Ctor default: 8.0
  EXPECT_EQ(8.0, awhp.ratedInletAirTemperature());
  EXPECT_TRUE(awhp.setRatedInletAirTemperature(0.4));
  EXPECT_EQ(0.4, awhp.ratedInletAirTemperature());

  // Rated Air Flow Rate: Required Double
  // Ctor default: Autosize
  EXPECT_TRUE(awhp.isRatedAirFlowRateAutosized());
  // Set
  EXPECT_TRUE(awhp.setRatedAirFlowRate(0.5));
  ASSERT_TRUE(awhp.ratedAirFlowRate());
  EXPECT_EQ(0.5, awhp.ratedAirFlowRate().get());
  // Bad Value
  EXPECT_FALSE(awhp.setRatedAirFlowRate(-10.0));
  ASSERT_TRUE(awhp.ratedAirFlowRate());
  EXPECT_EQ(0.5, awhp.ratedAirFlowRate().get());
  EXPECT_FALSE(awhp.isRatedAirFlowRateAutosized());
  // Autosize
  awhp.autosizeRatedAirFlowRate();
  EXPECT_TRUE(awhp.isRatedAirFlowRateAutosized());

  // Rated Leaving Water Temperature: Required Double
  // Ctor default: 40.0
  EXPECT_EQ(40.0, awhp.ratedLeavingWaterTemperature());
  EXPECT_TRUE(awhp.setRatedLeavingWaterTemperature(0.6));
  EXPECT_EQ(0.6, awhp.ratedLeavingWaterTemperature());

  // Rated Water Flow Rate: Required Double
  // Ctor default: Autosize
  EXPECT_TRUE(awhp.isRatedWaterFlowRateAutosized());
  // Set
  EXPECT_TRUE(awhp.setRatedWaterFlowRate(0.7));
  ASSERT_TRUE(awhp.ratedWaterFlowRate());
  EXPECT_EQ(0.7, awhp.ratedWaterFlowRate().get());
  // Bad Value
  EXPECT_FALSE(awhp.setRatedWaterFlowRate(-10.0));
  ASSERT_TRUE(awhp.ratedWaterFlowRate());
  EXPECT_EQ(0.7, awhp.ratedWaterFlowRate().get());
  EXPECT_FALSE(awhp.isRatedWaterFlowRateAutosized());
  // Autosize
  awhp.autosizeRatedWaterFlowRate();
  EXPECT_TRUE(awhp.isRatedWaterFlowRateAutosized());

  // Minimum Outdoor Air Temperature: Required Double
  // Ctor default: -30.0
  EXPECT_EQ(-30.0, awhp.minimumOutdoorAirTemperature());
  EXPECT_TRUE(awhp.setMinimumOutdoorAirTemperature(0.8));
  EXPECT_EQ(0.8, awhp.minimumOutdoorAirTemperature());

  // Maximum Outdoor Air Temperature: Required Double
  // Ctor default: 100.0
  EXPECT_EQ(100.0, awhp.maximumOutdoorAirTemperature());
  EXPECT_TRUE(awhp.setMaximumOutdoorAirTemperature(0.9));
  EXPECT_EQ(0.9, awhp.maximumOutdoorAirTemperature());

  // Minimum Leaving Water Temperature Curve Name: Optional Object
  CurveCubic minimumLeavingWaterTemperatureCurve(m);
  EXPECT_TRUE(awhp.setMinimumLeavingWaterTemperatureCurve(minimumLeavingWaterTemperatureCurve));
  ASSERT_TRUE(awhp.minimumLeavingWaterTemperatureCurve());
  EXPECT_EQ(minimumLeavingWaterTemperatureCurve, awhp.minimumLeavingWaterTemperatureCurve().get());
  awhp.resetMinimumLeavingWaterTemperatureCurve();
  EXPECT_FALSE(awhp.minimumLeavingWaterTemperatureCurve());

  // Maximum Leaving Water Temperature Curve Name: Optional Object
  CurveCubic maximumLeavingWaterTemperatureCurve(m);
  EXPECT_TRUE(awhp.setMaximumLeavingWaterTemperatureCurve(maximumLeavingWaterTemperatureCurve));
  ASSERT_TRUE(awhp.maximumLeavingWaterTemperatureCurve());
  EXPECT_EQ(maximumLeavingWaterTemperatureCurve, awhp.maximumLeavingWaterTemperatureCurve().get());

  // Sizing Factor: Required Double
  // Ctor default: 1.0
  EXPECT_EQ(1.0, awhp.sizingFactor());
  // Set
  EXPECT_TRUE(awhp.setSizingFactor(1.2));
  EXPECT_EQ(1.2, awhp.sizingFactor());
  // Bad Value
  EXPECT_FALSE(awhp.setSizingFactor(-10.0));
  EXPECT_EQ(1.2, awhp.sizingFactor());

  // Booster Mode On Speed: Optional Object
  HeatPumpAirToWaterHeatingSpeedData boosterModeOnSpeed(m);
  EXPECT_TRUE(awhp.setBoosterModeOnSpeed(boosterModeOnSpeed));
  ASSERT_TRUE(awhp.boosterModeOnSpeed());
  EXPECT_EQ(boosterModeOnSpeed, awhp.boosterModeOnSpeed().get());
  awhp.resetBoosterModeOnSpeed();
  EXPECT_FALSE(awhp.boosterModeOnSpeed());
}

TEST_F(ModelFixture, HeatPumpAirToWaterHeating_HeatCoolFuelTypes) {
  Model m;
  HeatPumpAirToWaterHeating awhp(m);

  EXPECT_EQ(ComponentType(ComponentType::Heating), awhp.componentType());
  testFuelTypeEquality({}, awhp.coolingFuelTypes());
  testFuelTypeEquality({FuelType::Electricity, FuelType::Propane}, awhp.heatingFuelTypes());
  testAppGFuelTypeEquality({AppGFuelType::HeatPump}, awhp.appGHeatingFuelTypes());
}
