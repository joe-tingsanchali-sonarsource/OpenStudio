within OpenStudioExample.TemplatesRTUs;
model RTU_Ideal "A model for an RTU providing ideal heating and cooling"
  extends OpenStudioExample.TemplatesRTUs.Interfaces.RTU;
  Buildings.Controls.OBC.CDL.Reals.Sources.Constant TSetCooCon(k=TSetCoo) "Set point temperature for cooling"
    annotation (Placement(transformation(extent={{-60,30},{-40,50}})));
  Buildings.ThermalZones.EnergyPlus_24_2_0.Examples.SmallOffice.BaseClasses.IdealHeaterCooler
      coo(Q_flow_nominal=-QCoo_flow_nominal)
    "Ideal cooling device for sensible cooling"
    annotation (Placement(transformation(rotation=0,extent={{-20,30},{0,50}})));
  Buildings.Controls.OBC.CDL.Reals.Sources.Constant TSetHeaCon(k=TSetHea) "Set point temperature for heating"
    annotation (Placement(transformation(extent={{-60,-50},{-40,-30}})));
  Buildings.ThermalZones.EnergyPlus_24_2_0.Examples.SmallOffice.BaseClasses.IdealHeaterCooler
      hea(Q_flow_nominal=QHea_flow_nominal)
    "Ideal heating device for sensible heating" annotation (Placement(
        transformation(rotation=0, extent={{-20,-50},{0,-30}})));
equation
  connect(TSetCooCon.y, coo.TSet)
    annotation (Line(points={{-38,40},{-22,40}}, color={0,0,127}));
  connect(TSetHeaCon.y, hea.TSet)
    annotation (Line(points={{-38,-40},{-22,-40}}, color={0,0,127}));
  connect(TMea, coo.TMea)
    annotation (Line(points={{-220,0},{-10,0},{-10,28}}, color={0,0,127}));
  connect(TMea, hea.TMea) annotation (Line(points={{-220,0},{-80,0},{-80,-60},{-10,
          -60},{-10,-52}}, color={0,0,127}));
  connect(coo.heaPor, heaPor)
    annotation (Line(points={{0,40},{200,40},{200,0}},       color={191,0,0}));
  connect(hea.heaPor, heaPor) annotation (Line(points={{0,-40},{200,-40},{200,0}},
                   color={191,0,0}));
  annotation (Icon(coordinateSystem(preserveAspectRatio=false, extent={{-200,-200},
            {200,200}}),                                        graphics={
        Text(
          extent={{-150,240},{150,200}},
          textString="%name",
          textColor={0,0,255}),
        Rectangle(
          extent={{-120,-20},{120,-142}},
          lineColor={0,0,0},
          fillColor={238,46,47},
          fillPattern=FillPattern.Solid),
        Rectangle(
          extent={{-120,140},{120,20}},
          lineColor={0,0,0},
          fillColor={28,108,200},
          fillPattern=FillPattern.Solid)}),                      Diagram(
        coordinateSystem(preserveAspectRatio=false, extent={{-200,-200},{200,200}})));
end RTU_Ideal;
