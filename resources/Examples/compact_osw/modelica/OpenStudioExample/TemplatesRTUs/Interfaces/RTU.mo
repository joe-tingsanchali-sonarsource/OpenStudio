within OpenStudioExample.TemplatesRTUs.Interfaces;
partial model RTU "An interface model for a single zone RTU"
  parameter Modelica.Units.SI.Temperature TSetCoo "Temperature set point for cooling";
  parameter Modelica.Units.SI.Temperature TSetHea "Temperature set point for heating";
  parameter Modelica.Units.SI.Power QCoo_flow_nominal "Nominal cooling capacity (>0)";
  parameter Modelica.Units.SI.Power QHea_flow_nominal "Nominal heating capacity (>0)";
  Modelica.Blocks.Interfaces.RealInput TMea "Measured temperature"
    annotation (Placement(transformation(extent={{-240,-20},{-200,20}})));
  Modelica.Thermal.HeatTransfer.Interfaces.HeatPort_b heaPor "Heat port"
    annotation (Placement(transformation(extent={{190,-10},{210,10}})));
  annotation (Icon(coordinateSystem(preserveAspectRatio=false, extent={{-200,-200},
            {200,200}}),                                        graphics={
        Text(
          extent={{-150,240},{150,200}},
          textString="%name",
          textColor={0,0,255}), Rectangle(extent={{-200,200},{200,-200}},
            lineColor={0,0,0})}),                                Diagram(
        coordinateSystem(preserveAspectRatio=false, extent={{-200,-200},{200,200}})));
end RTU;
