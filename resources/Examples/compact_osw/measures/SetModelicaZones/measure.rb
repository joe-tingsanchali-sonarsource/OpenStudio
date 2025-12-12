
# insert your copyright here

# see the URL below for information on how to write OpenStudio measures
# http://nrel.github.io/OpenStudio-user-documentation/reference/measure_writing_guide/

# start the measure
class SetModelicaZones < OpenStudio::Measure::ModelicaMeasure
  # human readable name
  def name
    # Measure name should be the title case of the class name.
    return 'SetModelicaZones'
  end

  # human readable description
  def description
    return 'DESCRIPTION_TEXT'
  end

  # human readable description of modeling approach
  def modeler_description
    return 'MODELER_DESCRIPTION_TEXT'
  end

  # define the arguments that the user will input
  def arguments(model, workspace)
    args = OpenStudio::Measure::OSArgumentVector.new
    return args
  end

  # define what happens when the measure is run
  def run(modelica_file, model, workspace, runner, user_arguments)
    super(modelica_file, model, workspace, runner, user_arguments)  # Do **NOT** remove this line

    if !runner.validateUserArguments(arguments(model, workspace), user_arguments)
      return false
    end

    all_zones = model.getThermalZones()

    main_class_definition = modelica_file.getClassDefinitions().first()

    all_zones.each do |zone|
      zone_name = zone.nameString

      if plenum_zone?(zone)
        runner.registerInfo("Skipping Modelica zone for '#{zone_name}' because its space type is Plenum.")
        next
      end

      component_name = modelica_identifier(zone_name)
      main_class_definition.addComponentClause(
        <<-COMPCLAUSE

  OpenStudioExample.TemplatesZoneHVAC.SingleZoneRTU #{component_name}(
    redeclare OpenStudioExample.TemplatesRTUs.RTU_Ideal rtu(
      TSetHea=273.15 + 20,
      TSetCoo=273.15 + 25,
      QHea_flow_nominal=10000,
      QCoo_flow_nominal=10000),
    zon(zoneName=\"#{zone_name}\"));
        COMPCLAUSE
      )
    end

    return true
  end

  private

  def plenum_zone?(zone)
    zone.spaces.any? do |space|
      space_type = space.spaceType
      next false unless space_type.is_initialized

      st = space_type.get
      standards_type = st.standardsSpaceType
      if standards_type.is_initialized
        standards_type.get.casecmp('Plenum').zero?
      else
        st.nameString.casecmp('Plenum').zero?
      end
    end
  end

  def modelica_identifier(name)
    sanitized = name.gsub(/[^0-9A-Za-z_]/, '_')
    sanitized = "zone_#{sanitized}" if sanitized.empty? || sanitized[0] =~ /\d/
    sanitized
  end
end

# register the measure to be used by the application
SetModelicaZones.new.registerWithApplication
