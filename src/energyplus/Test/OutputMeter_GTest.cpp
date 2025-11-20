/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>
#include "EnergyPlusFixture.hpp"

#include "../ForwardTranslator.hpp"
#include "../ReverseTranslator.hpp"

#include "../../model/OutputMeter.hpp"
#include "../../model/OutputMeter_Impl.hpp"

#include "../../utilities/idf/Workspace.hpp"
#include "../../utilities/idf/IdfObject.hpp"
#include "../../utilities/idf/WorkspaceObject.hpp"
#include "../../utilities/data/DataEnums.hpp"

// E+ FieldEnums
#include <utilities/idd/IddEnums.hxx>
#include <utilities/idd/IddFactory.hxx>
#include <utilities/idd/Output_Meter_FieldEnums.hxx>
#include <utilities/idd/Output_Meter_MeterFileOnly_FieldEnums.hxx>
#include <utilities/idd/Output_Meter_Cumulative_FieldEnums.hxx>
#include <utilities/idd/Output_Meter_Cumulative_MeterFileOnly_FieldEnums.hxx>

using namespace openstudio::energyplus;
using namespace openstudio::model;
using namespace openstudio;

TEST_F(EnergyPlusFixture, ForwardTranslator_OutputMeter_Regular) {

  ForwardTranslator ft;

  Model m;
  OutputMeter meter(m);
  EXPECT_TRUE(meter.setInstallLocationType(InstallLocationType::Facility));
  EXPECT_TRUE(meter.setFuelType(FuelType::Gas));

  EXPECT_TRUE(meter.setReportingFrequency("Timestep"));
  EXPECT_TRUE(meter.setMeterFileOnly(false));
  EXPECT_TRUE(meter.setCumulative(false));

  const Workspace w = ft.translateModel(m);
  const auto idfObjs = w.getObjectsByType(IddObjectType::Output_Meter);
  ASSERT_EQ(1u, idfObjs.size());

  const auto& idfObject = idfObjs.front();
  EXPECT_EQ("NaturalGas:Facility", idfObject.getString(Output_MeterFields::KeyName).get());
  EXPECT_EQ("Timestep", idfObject.getString(Output_MeterFields::ReportingFrequency).get());
}

TEST_F(EnergyPlusFixture, ForwardTranslator_OutputMeter_MeterFileOnly) {

  ForwardTranslator ft;

  Model m;
  OutputMeter meter(m);
  EXPECT_TRUE(meter.setInstallLocationType(InstallLocationType::Facility));
  EXPECT_TRUE(meter.setFuelType(FuelType::Gas));

  EXPECT_TRUE(meter.setReportingFrequency("Timestep"));
  EXPECT_TRUE(meter.setMeterFileOnly(true));
  EXPECT_TRUE(meter.setCumulative(false));

  const Workspace w = ft.translateModel(m);
  const auto idfObjs = w.getObjectsByType(IddObjectType::Output_Meter_MeterFileOnly);
  ASSERT_EQ(1u, idfObjs.size());

  const auto& idfObject = idfObjs.front();
  EXPECT_EQ("NaturalGas:Facility", idfObject.getString(Output_Meter_MeterFileOnlyFields::KeyName).get());
  EXPECT_EQ("Timestep", idfObject.getString(Output_Meter_MeterFileOnlyFields::ReportingFrequency).get());
}

TEST_F(EnergyPlusFixture, ForwardTranslator_OutputMeter_Cumulative) {

  ForwardTranslator ft;

  Model m;
  OutputMeter meter(m);
  EXPECT_TRUE(meter.setInstallLocationType(InstallLocationType::Facility));
  EXPECT_TRUE(meter.setFuelType(FuelType::Gas));

  EXPECT_TRUE(meter.setReportingFrequency("Timestep"));
  EXPECT_TRUE(meter.setMeterFileOnly(false));
  EXPECT_TRUE(meter.setCumulative(true));

  const Workspace w = ft.translateModel(m);
  const auto idfObjs = w.getObjectsByType(IddObjectType::Output_Meter_Cumulative);
  ASSERT_EQ(1u, idfObjs.size());

  const auto& idfObject = idfObjs.front();
  EXPECT_EQ("NaturalGas:Facility", idfObject.getString(Output_Meter_CumulativeFields::KeyName).get());
  EXPECT_EQ("Timestep", idfObject.getString(Output_Meter_CumulativeFields::ReportingFrequency).get());
}

TEST_F(EnergyPlusFixture, ForwardTranslator_OutputMeter_Cumulative_MeterFileOnly) {

  ForwardTranslator ft;

  Model m;
  OutputMeter meter(m);
  EXPECT_TRUE(meter.setInstallLocationType(InstallLocationType::Facility));
  EXPECT_TRUE(meter.setFuelType(FuelType::Gas));

  EXPECT_TRUE(meter.setReportingFrequency("Timestep"));
  EXPECT_TRUE(meter.setMeterFileOnly(true));
  EXPECT_TRUE(meter.setCumulative(true));

  const Workspace w = ft.translateModel(m);
  const auto idfObjs = w.getObjectsByType(IddObjectType::Output_Meter_Cumulative_MeterFileOnly);
  ASSERT_EQ(1u, idfObjs.size());

  const auto& idfObject = idfObjs.front();
  EXPECT_EQ("NaturalGas:Facility", idfObject.getString(Output_Meter_Cumulative_MeterFileOnlyFields::KeyName).get());
  EXPECT_EQ("Timestep", idfObject.getString(Output_Meter_Cumulative_MeterFileOnlyFields::ReportingFrequency).get());
}

TEST_F(EnergyPlusFixture, ReverseTranslator_OutputMeter_Regular) {

  ReverseTranslator rt;

  Workspace w(StrictnessLevel::Minimal, IddFileType::EnergyPlus);

  auto woMeter = w.addObject(IdfObject(IddObjectType::Output_Meter)).get();

  EXPECT_TRUE(woMeter.setString(Output_MeterFields::KeyName, "NaturalGas:Facility"));
  EXPECT_TRUE(woMeter.setString(Output_MeterFields::ReportingFrequency, "Timestep"));

  const Model m = rt.translateWorkspace(w);
  const auto modelObjects = m.getConcreteModelObjects<OutputMeter>();
  ASSERT_EQ(1u, modelObjects.size());

  const auto& modelObject = modelObjects.front();
  EXPECT_EQ("NaturalGas:Facility", modelObject.name());
  EXPECT_EQ("Timestep", modelObject.reportingFrequency());
  EXPECT_FALSE(modelObject.meterFileOnly());
  EXPECT_FALSE(modelObject.cumulative());
}

TEST_F(EnergyPlusFixture, ReverseTranslator_OutputMeter_MeterFileOnly) {

  ReverseTranslator rt;

  Workspace w(StrictnessLevel::Minimal, IddFileType::EnergyPlus);

  auto woMeter = w.addObject(IdfObject(IddObjectType::Output_Meter_MeterFileOnly)).get();

  EXPECT_TRUE(woMeter.setString(Output_Meter_MeterFileOnlyFields::KeyName, "NaturalGas:Facility"));
  EXPECT_TRUE(woMeter.setString(Output_Meter_MeterFileOnlyFields::ReportingFrequency, "Timestep"));

  const Model m = rt.translateWorkspace(w);
  const auto modelObjects = m.getConcreteModelObjects<OutputMeter>();
  ASSERT_EQ(1u, modelObjects.size());

  const auto& modelObject = modelObjects.front();
  EXPECT_EQ("NaturalGas:Facility", modelObject.name());
  EXPECT_EQ("Timestep", modelObject.reportingFrequency());
  EXPECT_TRUE(modelObject.meterFileOnly());
  EXPECT_FALSE(modelObject.cumulative());
}

TEST_F(EnergyPlusFixture, ReverseTranslator_OutputMeter_Cumulative) {

  ReverseTranslator rt;

  Workspace w(StrictnessLevel::Minimal, IddFileType::EnergyPlus);

  auto woMeter = w.addObject(IdfObject(IddObjectType::Output_Meter_Cumulative)).get();

  EXPECT_TRUE(woMeter.setString(Output_Meter_CumulativeFields::KeyName, "NaturalGas:Facility"));
  EXPECT_TRUE(woMeter.setString(Output_Meter_CumulativeFields::ReportingFrequency, "Timestep"));

  const Model m = rt.translateWorkspace(w);
  const auto modelObjects = m.getConcreteModelObjects<OutputMeter>();
  ASSERT_EQ(1u, modelObjects.size());

  const auto& modelObject = modelObjects.front();
  EXPECT_EQ("NaturalGas:Facility", modelObject.name());
  EXPECT_EQ("Timestep", modelObject.reportingFrequency());
  EXPECT_FALSE(modelObject.meterFileOnly());
  EXPECT_TRUE(modelObject.cumulative());
}

TEST_F(EnergyPlusFixture, ReverseTranslator_OutputMeter_Cumulative_MeterFileOnly) {

  ReverseTranslator rt;

  Workspace w(StrictnessLevel::Minimal, IddFileType::EnergyPlus);

  auto woMeter = w.addObject(IdfObject(IddObjectType::Output_Meter_Cumulative_MeterFileOnly)).get();

  EXPECT_TRUE(woMeter.setString(Output_Meter_Cumulative_MeterFileOnlyFields::KeyName, "NaturalGas:Facility"));
  EXPECT_TRUE(woMeter.setString(Output_Meter_Cumulative_MeterFileOnlyFields::ReportingFrequency, "Timestep"));

  const Model m = rt.translateWorkspace(w);
  const auto modelObjects = m.getConcreteModelObjects<OutputMeter>();
  ASSERT_EQ(1u, modelObjects.size());

  const auto& modelObject = modelObjects.front();
  EXPECT_EQ("NaturalGas:Facility", modelObject.name());
  EXPECT_EQ("Timestep", modelObject.reportingFrequency());
  EXPECT_TRUE(modelObject.meterFileOnly());
  EXPECT_TRUE(modelObject.cumulative());
}
