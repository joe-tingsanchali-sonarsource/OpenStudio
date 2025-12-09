/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>
#include "EnergyPlusFixture.hpp"

#include "../ForwardTranslator.hpp"

#include "../../model/Model.hpp"
#include "../../model/CoilWaterHeatingAirToWaterHeatPumpWrapped.hpp"
#include "../../model/CoilWaterHeatingAirToWaterHeatPumpWrapped_Impl.hpp"
#include "../../model/WaterHeaterHeatPumpWrappedCondenser.hpp"
#include "../../model/ThermalZone.hpp"
#include "../../model/Space.hpp"
#include "../../model/ScheduleConstant.hpp"
#include "../../model/ScheduleConstant_Impl.hpp"
#include "../../model/WaterHeaterStratified.hpp"
#include "../../model/WaterHeaterStratified_Impl.hpp"
#include "../../model/FanOnOff.hpp"
#include "../../model/FanOnOff_Impl.hpp"

#include <utilities/idd/WaterHeater_HeatPump_WrappedCondenser_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::energyplus;
using namespace openstudio::model;
using namespace openstudio;

TEST_F(EnergyPlusFixture, ForwardTranslator_WaterHeaterHeatPumpWrappedCondenser) {
  Model m;

  WaterHeaterStratified wh(m);
  CoilWaterHeatingAirToWaterHeatPumpWrapped coil(m);
  FanOnOff fan(m);

  WaterHeaterHeatPumpWrappedCondenser hpwh(m);

  hpwh.setName("My WaterHeaterHeatPumpWrappedCondenser");
  Schedule availabilitySchedule = m.alwaysOnDiscreteSchedule();
  EXPECT_TRUE(hpwh.setAvailabilitySchedule(availabilitySchedule));
  ScheduleConstant scheduleConstant1(m);
  scheduleConstant1.setValue(0.1);
  EXPECT_TRUE(hpwh.setCompressorSetpointTemperatureSchedule(scheduleConstant1));
  EXPECT_TRUE(hpwh.setDeadBandTemperatureDifference(1));
  EXPECT_TRUE(hpwh.setCondenserBottomLocation(2));
  EXPECT_TRUE(hpwh.setCondenserTopLocation(3));
  EXPECT_TRUE(hpwh.setEvaporatorAirFlowRate(4));
  EXPECT_TRUE(hpwh.setInletAirConfiguration("Schedule"));
  EXPECT_TRUE(hpwh.setAirInletNodeName(hpwh.nameString() + " Air Inlet Node"));
  EXPECT_TRUE(hpwh.setAirOutletNodeName(hpwh.nameString() + " Air Outlet Node"));
  ScheduleConstant scheduleConstant2(m);
  scheduleConstant2.setValue(0.2);
  EXPECT_TRUE(hpwh.setInletAirTemperatureSchedule(scheduleConstant2));
  ScheduleConstant scheduleConstant2(m);
  scheduleConstant2.setValue(0.2);
  EXPECT_TRUE(hpwh.setInletAirHumiditySchedule(scheduleConstant2));
  EXPECT_TRUE(hpwh.setTank(wh));
  EXPECT_TRUE(hpwh.setDXCoil(coil));
  EXPECT_TRUE(hpwh.setMinimumInletAirTemperatureforCompressorOperation(15));
  EXPECT_TRUE(hpwh.setMaximumInletAirTemperatureforCompressorOperation(30));
  EXPECT_TRUE(hpwh.setCompressorLocation("Outdoors"));
  ScheduleConstant scheduleConstant4(m);
  scheduleConstant4.setValue(0.4);
  EXPECT_TRUE(hpwh.setCompressorAmbientTemperatureSchedule(scheduleConstant4));
  EXPECT_TRUE(hpwh.setFan(fan));
  EXPECT_TRUE(hpwh.setFanPlacement("DrawThrough"));
  EXPECT_TRUE(hpwh.setOnCycleParasiticElectricLoad(40));
  EXPECT_TRUE(hpwh.setOffCycleParasiticElectricLoad(50));
  EXPECT_TRUE(hpwh.setParasiticHeatRejectionLocation("Zone"));
  ScheduleConstant scheduleConstant5(m);
  scheduleConstant5.setValue(0.5);
  EXPECT_TRUE(hpwh.setInletAirMixerSchedule(scheduleConstant5));
  EXPECT_TRUE(hpwh.setTankElementControlLogic("Simultaneous"));
  EXPECT_TRUE(hpwh.setControlSensor1HeightInStratifiedTank(60));
  EXPECT_TRUE(hpwh.setControlSensor1Weight(0.75));
  EXPECT_TRUE(hpwh.setControlSensor2HeightInStratifiedTank(70));

  ThermalZone tz(m);
  Space space(m);
  space.setThermalZone(tz);

  hpwh.addToThermalZone(tz);

  ForwardTranslator ft;
  const Workspace w = ft.translateModel(m);

  const auto idfObjs = w.getObjectsByType(IddObjectType::WaterHeater_HeatPump_WrappedCondenser);
  ASSERT_EQ(1u, idfObjs.size());
  const auto& idfObject = idfObjs.front();

  EXPECT_EQ(hpwh.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::Name).get());
  EXPECT_EQ(availabilitySchedule.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::AvailabilityScheduleName).get());
  EXPECT_EQ(scheduleConstant1.nameString(),
            idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::CompressorSetpointTemperatureScheduleName).get());
  EXPECT_EQ(1, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::DeadBandTemperatureDifference).get());
  EXPECT_EQ(2, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::CondenserBottomLocation).get());
  EXPECT_EQ(3, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::CondenserTopLocation).get());
  EXPECT_EQ(4, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::EvaporatorAirFlowRate).get());
  EXPECT_EQ("Schedule", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirConfiguration).get());
  EXPECT_EQ("HPWH Air Inlet Node", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::AirInletNodeName).get());
  EXPECT_EQ("HPWH Air Outlet Node", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::AirOutletNodeName).get());
  EXPECT_EQ(idfObject.isEmpty(WaterHeater_HeatPump_WrappedCondenserFields::OutdoorAirNodeName));
  EXPECT_EQ(idfObject.isEmpty(WaterHeater_HeatPump_WrappedCondenserFields::ExhaustAirNodeName));
  EXPECT_EQ(scheduleConstant2.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirTemperatureScheduleName).get());
  EXPECT_EQ(scheduleConstant3.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirHumidityScheduleName).get());
  EXPECT_EQ(tz.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirZoneName).get());
  EXPECT_EQ("WaterHeater:Stratified", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::TankObjectType).get());
  EXPECT_EQ(wh.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::TankName).get());
  EXPECT_EQ(wh.supplyInletModelObject()->name().get(),
            idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::TankUseSideInletNodeName).get());
  EXPECT_EQ(wh.supplyOutletModelObject()->name().get(),
            idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::TankUseSideOutletNodeName).get());
  EXPECT_EQ("Coil:WaterHeating:AirToWaterHeatPump:Wrapped", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::DXCoilObjectType).get());
  EXPECT_EQ(coil.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::DXCoilName).get());
  EXPECT_EQ(15, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::MinimumInletAirTemperatureforCompressorOperation).get());
  EXPECT_EQ(30, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::MaximumInletAirTemperatureforCompressorOperation).get());
  EXPECT_EQ("Outdoors", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::CompressorLocation).get());
  EXPECT_EQ(scheduleConstant4.nameString(),
            idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::CompressorAmbientTemperatureScheduleName).get());
  EXPECT_EQ("Fan:OnOff", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::FanObjectType).get());
  EXPECT_EQ(fan.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::FanName).get());
  EXPECT_EQ("DrawThrough", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::FanPlacement).get());
  EXPECT_EQ(40, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::OnCycleParasiticElectricLoad).get());
  EXPECT_EQ(50, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::OffCycleParasiticElectricLoad).get());
  EXPECT_EQ("Zone", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::ParasiticHeatRejectionLocation).get());
  EXPECT_EQ(idfObject.isEmpty(WaterHeater_HeatPump_WrappedCondenserFields::InletAirMixerNodeName));
  EXPECT_EQ(idfObject.isEmpty(WaterHeater_HeatPump_WrappedCondenserFields::OutletAirSplitterNodeName));
  EXPECT_EQ(scheduleConstant5.nameString(), idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::InletAirMixerScheduleName).get());
  EXPECT_EQ("Simultaneous", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::TankElementControlLogic).get());
  EXPECT_EQ(60, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::ControlSensor1HeightInStratifiedTank).get());
  EXPECT_EQ(0.75, idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::ControlSensor1Weight).get());
  EXPECT_EQ(70, idfObject.getDouble(WaterHeater_HeatPump_WrappedCondenserFields::ControlSensor2HeightInStratifiedTank).get());
}
