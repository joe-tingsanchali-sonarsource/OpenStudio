within OpenStudioExample.TemplatesZoneHVAC;
model SingleZoneRTU "A template model for single zone RTU"
  Buildings.ThermalZones.EnergyPlus_24_2_0.ThermalZone zon(redeclare package
      Medium = Buildings.Media.Air) "Zone model"
    annotation (Placement(transformation(extent={{40,-20},{80,20}})));
  replaceable OpenStudioExample.TemplatesRTUs.Interfaces.RTU rtu constrainedby
    OpenStudioExample.TemplatesRTUs.Interfaces.RTU
    "Single zone RTU model"
    annotation (Placement(transformation(extent={{-60,-20},{-20,20}})));
  Modelica.Blocks.Sources.Constant qIntGai[3](each k=0)
    "Internal heat gains, set to zero because these are modeled in EnergyPlus"
    annotation (Placement(transformation(extent={{0,10},{20,30}})));
equation
  connect(qIntGai.y, zon.qGai_flow) annotation (Line(points={{21,20},{30,20},{30,
          10},{38,10}}, color={0,0,127}));
  connect(zon.TAir, rtu.TMea) annotation (Line(points={{81,18},{90,18},{90,40},{
          -80,40},{-80,0},{-62,0}}, color={0,0,127}));
  connect(rtu.heaPor, zon.heaPorAir)
    annotation (Line(points={{-20,0},{60,0}}, color={191,0,0}));
  annotation (Icon(coordinateSystem(preserveAspectRatio=false), graphics={
        Rectangle(
          extent={{-100,100},{100,-100}},
          lineColor={0,0,0},
          fillColor={255,255,255},
          fillPattern=FillPattern.Solid),
        Text(
          extent={{-150,140},{150,100}},
          textString="%name",
          textColor={0,0,255}),
        Rectangle(
          extent={{-58,16},{22,-64}},
          lineColor={0,0,0},
          fillColor={215,215,215},
          fillPattern=FillPattern.Solid),
        Polygon(
          points={{-18,56},{62,56},{22,16},{-58,16},{-18,56}},
          lineColor={0,0,0},
          fillColor={175,175,175},
          fillPattern=FillPattern.Solid),
        Polygon(
          points={{62,56},{62,-24},{22,-64},{22,16},{62,56}},
          lineColor={0,0,0},
          fillColor={135,135,135},
          fillPattern=FillPattern.Solid),
        Rectangle(
          extent={{-38,-4},{2,-44}},
          lineColor={0,0,0},
          fillColor={170,213,255},
          fillPattern=FillPattern.Solid),
        Rectangle(
          extent={{-22,48},{18,28}},
          lineColor={0,0,0},
          fillColor={255,213,170},
          fillPattern=FillPattern.Solid),
        Polygon(
          points={{-22,48},{-10,60},{30,60},{18,48},{-22,48}},
          lineColor={0,0,0},
          fillColor={255,170,85},
          fillPattern=FillPattern.Solid),
        Polygon(
          points={{30,60},{30,40},{18,28},{18,48},{30,60}},
          lineColor={0,0,0},
          fillColor={255,128,0},
          fillPattern=FillPattern.Solid)}),                      Diagram(
        coordinateSystem(preserveAspectRatio=false)));
end SingleZoneRTU;
