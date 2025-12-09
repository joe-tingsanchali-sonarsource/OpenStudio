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

#include <utilities/idd/WaterHeater_HeatPump_WrappedCondenser_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::energyplus;
using namespace openstudio::model;
using namespace openstudio;

TEST_F(EnergyPlusFixture, ForwardTranslator_CoilWaterHeatingAirToWaterHeatPumpWrapped) {
  Model m;

  CoilWaterHeatingAirToWaterHeatPumpWrapped coil(m);

  WaterHeaterHeatPumpWrappedCondenser hpwh(m);
  hpwh.setName("HPWH");
  EXPECT_TRUE(hpwh.setAirInletNodeName(hpwh.nameString() + " Air Inlet Node"));
  EXPECT_TRUE(hpwh.setAirOutletNodeName(hpwh.nameString() + " Air Outlet Node"));
  ThermalZone tz(m);
  Space space(m);
  space.setThermalZone(tz);

  hpwh.setDXCoil(coil);
  hpwh.addToThermalZone(tz);

  ForwardTranslator ft;
  const Workspace w = ft.translateModel(m);

  auto idfs_hpwhs = w.getObjectsByType(IddObjectType::WaterHeater_HeatPump_WrappedCondenser);
  ASSERT_EQ(1, idfs_hpwhs.size());
  const WorkspaceObject& idfs_hpwh = idfs_hpwhs[0];

  EXPECT_EQ("HPWH", idf_coil.getString(WaterHeater_HeatPump_WrappedCondenserFields::Name).get());
  EXPECT_EQ("HPWH Air Inlet Node", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::AirInletNodeName).get());
  EXPECT_EQ("HPWH Air Outlet Node", idfObject.getString(WaterHeater_HeatPump_WrappedCondenserFields::AirOutletNodeName).get());
}
