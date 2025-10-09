/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include "ModelFixture.hpp"

#include "../HeatPumpAirToWater.hpp"
#include "../HeatPumpAirToWater_Impl.hpp"

#include "../Schedule.hpp"
#include "../ScheduleConstant.hpp"

#include "../Curve.hpp"
#include "../CurveCubic.hpp"
#include "../CurveBicubic.hpp"

#include "../AirLoopHVAC.hpp"
#include "../Node.hpp"
#include "../PlantLoop.hpp"

#include "../HeatPumpAirToWaterHeating.hpp"
#include "../HeatPumpAirToWaterHeatingSpeedData.hpp"

#include "../HeatPumpAirToWaterCooling.hpp"
#include "../HeatPumpAirToWaterCoolingSpeedData.hpp"

using namespace openstudio;
using namespace openstudio::model;

TEST_F(ModelFixture, HeatPumpAirToWater_GettersSetters) {
  Model m;
  HeatPumpAirToWater heatPumpAirToWater(m);

  heatPumpAirToWater.setName("My HeatPumpAirToWater");

  // Operating Mode Control Method: Required String
  // Ctor default
  EXPECT_EQ("Load", heatPumpAirToWater.operatingModeControlMethod());
  EXPECT_TRUE(heatPumpAirToWater.setOperatingModeControlMethod("ScheduledModes"));
  EXPECT_EQ("ScheduledModes", heatPumpAirToWater.operatingModeControlMethod());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setOperatingModeControlMethod("BADENUM"));
  EXPECT_EQ("ScheduledModes", heatPumpAirToWater.operatingModeControlMethod());

  // Operating Mode Control Option for Multiple Unit: Required String
  // Ctor default
  EXPECT_EQ("SingleMode", heatPumpAirToWater.operatingModeControlOptionforMultipleUnit());
  EXPECT_TRUE(heatPumpAirToWater.setOperatingModeControlOptionforMultipleUnit("CoolingPriority"));
  EXPECT_EQ("CoolingPriority", heatPumpAirToWater.operatingModeControlOptionforMultipleUnit());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setOperatingModeControlOptionforMultipleUnit("BADENUM"));
  EXPECT_EQ("CoolingPriority", heatPumpAirToWater.operatingModeControlOptionforMultipleUnit());

  // Operating Mode Control Schedule Name: Optional Object
  EXPECT_FALSE(heatPumpAirToWater.operatingModeControlSchedule());
  ScheduleConstant operatingModeControlSchedule(m);
  EXPECT_TRUE(heatPumpAirToWater.setOperatingModeControlSchedule(operatingModeControlSchedule));
  ASSERT_TRUE(heatPumpAirToWater.operatingModeControlSchedule());
  EXPECT_EQ(operatingModeControlSchedule, heatPumpAirToWater.operatingModeControlSchedule().get());

  // Minimum Part Load Ratio: Required Double
  // Ctor default
  EXPECT_EQ(0.0, heatPumpAirToWater.minimumPartLoadRatio());
  EXPECT_TRUE(heatPumpAirToWater.setMinimumPartLoadRatio(0.6));
  EXPECT_EQ(0.6, heatPumpAirToWater.minimumPartLoadRatio());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setMinimumPartLoadRatio(-10.0));
  EXPECT_EQ(0.6, heatPumpAirToWater.minimumPartLoadRatio());

  // Air Inlet Node Name: Optional String
  EXPECT_FALSE(heatPumpAirToWater.airInletNodeName());
  EXPECT_TRUE(heatPumpAirToWater.setAirInletNodeName("AWHP Air Inlet Node"));
  ASSERT_TRUE(heatPumpAirToWater.airInletNodeName());
  EXPECT_EQ("AWHP Air Inlet Node", heatPumpAirToWater.airInletNodeName().get());

  // Air Outlet Node Name: Optional String
  EXPECT_FALSE(heatPumpAirToWater.airOutletNodeName());
  EXPECT_TRUE(heatPumpAirToWater.setAirOutletNodeName("AWHP Air Outlet Node"));
  ASSERT_TRUE(heatPumpAirToWater.airOutletNodeName());
  EXPECT_EQ("AWHP Air Outlet Node", heatPumpAirToWater.airOutletNodeName().get());

  // Maximum Outdoor Dry Bulb Temperature For Defrost Operation: Required Double
  // Ctor default
  EXPECT_EQ(10.0, heatPumpAirToWater.maximumOutdoorDryBulbTemperatureForDefrostOperation());
  EXPECT_TRUE(heatPumpAirToWater.setMaximumOutdoorDryBulbTemperatureForDefrostOperation(0.9));
  EXPECT_EQ(0.9, heatPumpAirToWater.maximumOutdoorDryBulbTemperatureForDefrostOperation());

  // Heat Pump Defrost Control: Required String
  // Ctor default
  EXPECT_EQ("None", heatPumpAirToWater.heatPumpDefrostControl());
  EXPECT_TRUE(heatPumpAirToWater.setHeatPumpDefrostControl("TimedEmpirical"));
  EXPECT_EQ("TimedEmpirical", heatPumpAirToWater.heatPumpDefrostControl());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setHeatPumpDefrostControl("BADENUM"));
  EXPECT_EQ("TimedEmpirical", heatPumpAirToWater.heatPumpDefrostControl());

  // Heat Pump Defrost Time Period Fraction: Required Double
  // Ctor default
  EXPECT_EQ(0.058333, heatPumpAirToWater.heatPumpDefrostTimePeriodFraction());
  EXPECT_TRUE(heatPumpAirToWater.setHeatPumpDefrostTimePeriodFraction(1.1));
  EXPECT_EQ(1.1, heatPumpAirToWater.heatPumpDefrostTimePeriodFraction());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setHeatPumpDefrostTimePeriodFraction(-10.0));
  EXPECT_EQ(1.1, heatPumpAirToWater.heatPumpDefrostTimePeriodFraction());

  // Resistive Defrost Heater Capacity: Required Double
  // Ctor default
  ASSERT_TRUE(heatPumpAirToWater.resistiveDefrostHeaterCapacity());
  EXPECT_EQ(0.0, heatPumpAirToWater.resistiveDefrostHeaterCapacity().get());
  // Autosize
  heatPumpAirToWater.autosizeResistiveDefrostHeaterCapacity();
  EXPECT_TRUE(heatPumpAirToWater.isResistiveDefrostHeaterCapacityAutosized());
  // Set
  EXPECT_TRUE(heatPumpAirToWater.setResistiveDefrostHeaterCapacity(1.2));
  ASSERT_TRUE(heatPumpAirToWater.resistiveDefrostHeaterCapacity());
  EXPECT_EQ(1.2, heatPumpAirToWater.resistiveDefrostHeaterCapacity().get());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setResistiveDefrostHeaterCapacity(-10.0));
  ASSERT_TRUE(heatPumpAirToWater.resistiveDefrostHeaterCapacity());
  EXPECT_EQ(1.2, heatPumpAirToWater.resistiveDefrostHeaterCapacity().get());
  EXPECT_FALSE(heatPumpAirToWater.isResistiveDefrostHeaterCapacityAutosized());

  // Defrost Energy Input Ratio Function of Temperature Curve Name: Optional Object, BivariateFunctions
  CurveBicubic defrostEnergyInputRatioFunctionofTemperatureCurve(m);
  EXPECT_TRUE(heatPumpAirToWater.setDefrostEnergyInputRatioFunctionofTemperatureCurve(defrostEnergyInputRatioFunctionofTemperatureCurve));
  ASSERT_TRUE(heatPumpAirToWater.defrostEnergyInputRatioFunctionofTemperatureCurve());
  EXPECT_EQ(defrostEnergyInputRatioFunctionofTemperatureCurve, heatPumpAirToWater.defrostEnergyInputRatioFunctionofTemperatureCurve().get());
  {
    // Bad dimensions
    CurveCubic badCurve(m);
    EXPECT_FALSE(heatPumpAirToWater.setDefrostEnergyInputRatioFunctionofTemperatureCurve(badCurve));
    ASSERT_TRUE(heatPumpAirToWater.defrostEnergyInputRatioFunctionofTemperatureCurve());
    EXPECT_EQ(defrostEnergyInputRatioFunctionofTemperatureCurve, heatPumpAirToWater.defrostEnergyInputRatioFunctionofTemperatureCurve().get());
  }

  // Heat Pump Multiplier: Required Integer
  // Ctor default
  EXPECT_EQ(1, heatPumpAirToWater.heatPumpMultiplier());
  EXPECT_TRUE(heatPumpAirToWater.setHeatPumpMultiplier(14));
  EXPECT_EQ(14, heatPumpAirToWater.heatPumpMultiplier());

  // Control Type: Required String
  // Ctor default
  EXPECT_EQ("VariableSpeed", heatPumpAirToWater.controlType());
  EXPECT_TRUE(heatPumpAirToWater.setControlType("FixedSpeed"));
  EXPECT_EQ("FixedSpeed", heatPumpAirToWater.controlType());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setControlType("BADENUM"));
  EXPECT_EQ("FixedSpeed", heatPumpAirToWater.controlType());

  // Crankcase Heater Capacity: Required Double
  // Ctor default
  EXPECT_EQ(0.0, heatPumpAirToWater.crankcaseHeaterCapacity());
  EXPECT_TRUE(heatPumpAirToWater.setCrankcaseHeaterCapacity(1.6));
  EXPECT_EQ(1.6, heatPumpAirToWater.crankcaseHeaterCapacity());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setCrankcaseHeaterCapacity(-10.0));
  EXPECT_EQ(1.6, heatPumpAirToWater.crankcaseHeaterCapacity());

  // Crankcase Heater Capacity Function of Temperature Curve Name: Optional Object, UnivariateFunctions
  EXPECT_FALSE(heatPumpAirToWater.crankcaseHeaterCapacityFunctionofTemperatureCurve());
  CurveCubic crankcaseHeaterCapacityFunctionofTemperatureCurve(m);
  EXPECT_TRUE(heatPumpAirToWater.setCrankcaseHeaterCapacityFunctionofTemperatureCurve(crankcaseHeaterCapacityFunctionofTemperatureCurve));
  ASSERT_TRUE(heatPumpAirToWater.crankcaseHeaterCapacityFunctionofTemperatureCurve());
  EXPECT_EQ(crankcaseHeaterCapacityFunctionofTemperatureCurve, heatPumpAirToWater.crankcaseHeaterCapacityFunctionofTemperatureCurve().get());
  // Bad dimensions
  {
    CurveBicubic badCurve(m);
    EXPECT_FALSE(heatPumpAirToWater.setCrankcaseHeaterCapacityFunctionofTemperatureCurve(badCurve));
    ASSERT_TRUE(heatPumpAirToWater.crankcaseHeaterCapacityFunctionofTemperatureCurve());
    EXPECT_EQ(crankcaseHeaterCapacityFunctionofTemperatureCurve, heatPumpAirToWater.crankcaseHeaterCapacityFunctionofTemperatureCurve().get());
  }

  // Maximum Ambient Temperature for Crankcase Heater Operation: Required Double
  // Ctor default
  EXPECT_EQ(10.0, heatPumpAirToWater.maximumAmbientTemperatureforCrankcaseHeaterOperation());
  EXPECT_TRUE(heatPumpAirToWater.setMaximumAmbientTemperatureforCrankcaseHeaterOperation(1.8));
  EXPECT_EQ(1.8, heatPumpAirToWater.maximumAmbientTemperatureforCrankcaseHeaterOperation());
  // Bad Value
  EXPECT_FALSE(heatPumpAirToWater.setMaximumAmbientTemperatureforCrankcaseHeaterOperation(-10.0));
  EXPECT_EQ(1.8, heatPumpAirToWater.maximumAmbientTemperatureforCrankcaseHeaterOperation());

  // Heating Operation Mode: Optional Object
  HeatPumpAirToWaterHeating heatingOperationMode(m);
  EXPECT_TRUE(heatPumpAirToWater.setHeatingOperationMode(heatingOperationMode));
  ASSERT_TRUE(heatPumpAirToWater.heatingOperationMode());
  EXPECT_EQ(heatingOperationMode, heatPumpAirToWater.heatingOperationMode().get());

  // Cooling Operation Mode: Required Object
  HeatPumpAirToWaterCooling coolingOperationMode(m);
  EXPECT_TRUE(heatPumpAirToWater.setCoolingOperationMode(coolingOperationMode));
  EXPECT_EQ(coolingOperationMode, heatPumpAirToWater.coolingOperationMode().get());
}

TEST_F(ModelFixture, HeatPumpAirToWater_HeatCoolFuelTypes) {
  Model m;
  HeatPumpAirToWater heatPumpAirToWater(m);

  // No Cooling No Heating yet
  EXPECT_EQ(ComponentType(ComponentType::None), heatPumpAirToWater.componentType());
  testFuelTypeEquality({}, heatPumpAirToWater.coolingFuelTypes());
  testFuelTypeEquality({}, heatPumpAirToWater.heatingFuelTypes());
  testAppGFuelTypeEquality({}, heatPumpAirToWater.appGHeatingFuelTypes());

  HeatPumpAirToWaterCooling coolingOperationMode(m);
  EXPECT_TRUE(heatPumpAirToWater.setCoolingOperationMode(coolingOperationMode));
  EXPECT_EQ(ComponentType(ComponentType::Cooling), heatPumpAirToWater.componentType());
  testFuelTypeEquality({FuelType::Electricity}, heatPumpAirToWater.coolingFuelTypes());
  testFuelTypeEquality({}, heatPumpAirToWater.heatingFuelTypes());
  testAppGFuelTypeEquality({}, heatPumpAirToWater.appGHeatingFuelTypes());

  HeatPumpAirToWaterHeating heatingOperationMode(m);
  EXPECT_TRUE(heatPumpAirToWater.setHeatingOperationMode(heatingOperationMode));
  EXPECT_EQ(ComponentType(ComponentType::Both), heatPumpAirToWater.componentType());
  testFuelTypeEquality({FuelType::Electricity}, heatPumpAirToWater.coolingFuelTypes());
  testFuelTypeEquality({FuelType::Electricity}, heatPumpAirToWater.heatingFuelTypes());
  testAppGFuelTypeEquality({AppGFuelType::HeatPump}, heatPumpAirToWater.appGHeatingFuelTypes());

  heatPumpAirToWater.resetCoolingOperationMode();
  EXPECT_EQ(ComponentType(ComponentType::Heating), heatPumpAirToWater.componentType());
  testFuelTypeEquality({}, heatPumpAirToWater.coolingFuelTypes());
  testFuelTypeEquality({FuelType::Electricity}, heatPumpAirToWater.heatingFuelTypes());
  testAppGFuelTypeEquality({AppGFuelType::HeatPump}, heatPumpAirToWater.appGHeatingFuelTypes());
}

TEST_F(ModelFixture, HeatPumpAirToWater_Loop) {
  Model m;
  HeatPumpAirToWater awhp(m);
  HeatPumpAirToWaterCooling awhp_cc(m);
  EXPECT_TRUE(awhp.setCoolingOperationMode(awhp_cc));
  HeatPumpAirToWaterHeating awhp_hc(m);
  EXPECT_TRUE(awhp.setHeatingOperationMode(awhp_hc));

  PlantLoop chwLoop(m);
  {
    Node n = chwLoop.supplyInletNode();
    EXPECT_FALSE(awhp.addToNode(n));
  }
  {
    Node n = chwLoop.demandInletNode();
    EXPECT_FALSE(awhp.addToNode(n));
  }
  EXPECT_FALSE(chwLoop.addSupplyBranchForComponent(awhp));
  EXPECT_FALSE(chwLoop.addDemandBranchForComponent(awhp));

  AirLoopHVAC a(m);
  {
    Node n = a.supplyInletNode();
    EXPECT_FALSE(awhp.addToNode(n));
  }
  {
    Node n = a.demandInletNode();
    EXPECT_FALSE(awhp.addToNode(n));
  }

  EXPECT_FALSE(awhp.coolingLoop());
  EXPECT_FALSE(awhp.heatingLoop());

  EXPECT_TRUE(chwLoop.addSupplyBranchForComponent(awhp_cc));
  ASSERT_TRUE(awhp.coolingLoop());
  EXPECT_EQ(chwLoop, awhp.coolingLoop().get());
  EXPECT_FALSE(awhp.heatingLoop());

  PlantLoop hwLoop(m);
  EXPECT_TRUE(hwLoop.addSupplyBranchForComponent(awhp_hc));
  ASSERT_TRUE(awhp.coolingLoop());
  EXPECT_EQ(chwLoop, awhp.coolingLoop().get());
  ASSERT_TRUE(awhp.heatingLoop());
  EXPECT_EQ(hwLoop, awhp.heatingLoop().get());
}
