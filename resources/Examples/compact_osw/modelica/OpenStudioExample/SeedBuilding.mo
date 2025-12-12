within OpenStudioExample;
model SeedBuilding
  "A model to instantiate templates to represent a whole building"
  // This is the model the OS measure needs to create/modify

  // General parameters
  parameter String idfPath = "modelica://Buildings/Resources/Data/ThermalZones/EnergyPlus_24_2_0/Examples/SingleFamilyHouse_TwoSpeed_ZoneAirBalance/SingleFamilyHouse_TwoSpeed_ZoneAirBalance.idf" "Path to idf file";
  parameter String epwPath = "modelica://Buildings/Resources/weatherdata/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw" "Path to epw file";
  parameter String weaPath = "modelica://Buildings/Resources/weatherdata/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.mos" "Path to weather file";

  inner Buildings.ThermalZones.EnergyPlus_24_2_0.Building building(
    idfName=Modelica.Utilities.Files.loadResource(idfPath),
    epwName=Modelica.Utilities.Files.loadResource(epwPath),
    weaName=Modelica.Utilities.Files.loadResource(weaPath),
    usePrecompiledFMU=false,
    computeWetBulbTemperature=false) "Building model" annotation(
    Placement(transformation(extent={{-100,80},{-80,100}})));

end SeedBuilding;
