/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include "ModelFixture.hpp"

#include "../OutputMeter.hpp"
#include "../OutputMeter_Impl.hpp"

using namespace openstudio;
using namespace openstudio::model;

TEST_F(ModelFixture, OutputMeter_GettersSetters) {
  Model m;
  // TODO: Check regular Ctor arguments
  OutputMeter outputMeter(m);
  // TODO: Or if a UniqueModelObject (and make sure _Impl is included)
  // OutputMeter outputMeter = m.getUniqueModelObject<OutputMeter>();

  outputMeter.setName("My OutputMeter");

  // Reporting Frequency: Optional String
  // Default value from IDD
  EXPECT_TRUE(outputMeter.isReportingFrequencyDefaulted());
  EXPECT_EQ("Hourly", outputMeter.reportingFrequency());
  // Set
  EXPECT_TRUE(outputMeter.setReportingFrequency("Timestep"));
  EXPECT_EQ("Timestep", outputMeter.reportingFrequency());
  EXPECT_FALSE(outputMeter.isReportingFrequencyDefaulted());
  // Bad Value
  EXPECT_FALSE(outputMeter.setReportingFrequency("BADENUM"));
  EXPECT_EQ("Timestep", outputMeter.reportingFrequency());
  // Reset
  outputMeter.resetReportingFrequency();
  EXPECT_TRUE(outputMeter.isReportingFrequencyDefaulted());

  // Meter File Only: Optional Boolean
  // Default value from IDD
  EXPECT_TRUE(outputMeter.isMeterFileOnlyDefaulted());
  EXPECT_FALSE(outputMeter.meterFileOnly());
  EXPECT_TRUE(outputMeter.setMeterFileOnly(true));
  EXPECT_TRUE(outputMeter.meterFileOnly());
  EXPECT_TRUE(outputMeter.setMeterFileOnly(false));
  EXPECT_FALSE(outputMeter.meterFileOnly());

  // Cumulative: Optional Boolean
  // Default value from IDD
  EXPECT_TRUE(outputMeter.isCumulativeDefaulted());
  EXPECT_FALSE(outputMeter.cumulative());
  EXPECT_TRUE(outputMeter.setCumulative(true));
  EXPECT_TRUE(outputMeter.cumulative());
  EXPECT_TRUE(outputMeter.setCumulative(false));
  EXPECT_FALSE(outputMeter.cumulative());
}
