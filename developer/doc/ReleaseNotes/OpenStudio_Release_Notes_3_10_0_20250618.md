# OpenStudio Version 3.10.0

_Release Notes_ -  2025-06-18

These release notes describe version 3.10.0 of the OpenStudio SDK developed by the National Renewable Energy Laboratory (NREL), Buildings and Thermal Sciences Center, Commercial Buildings Research Group, Tools Development Section, and associated collaborators. The notes are organized into the following sections:

-  Overview
-  Where to Find OpenStudio Documentation
-  Installation Notes
-  OpenStudio SDK: Changelog

# Overview

As of April 2020, development and distribution of the OpenStudioApplication and the SketchUp plugin have transitioned to the OpenStudio Coalition, who is independently managing and distributing the software through its own [openstudiocoalition/OpenStudioApplication](https://github.com/openstudiocoalition/OpenStudioApplication) repository. The OpenStudio SDK is continuing to be actively developed and distributed by NREL and is released two times per year, through a spring and a fall release.

Below is the list of components that is included in this SDK installer:

__**OpenStudio SDK 3.10.0**__
- EnergyPlus
- Command Line Interface (CLI)
- Radiance
- Ruby API
- Python API
- C++ SDK

**Note** that [PAT](https://github.com/NREL/OpenStudio-PAT) is not included in either the SDK or the OpenStudio Coalition's Application installers. You will need to install PAT separately which is distributed on the [OpenStudio-PAT](https://github.com/NREL/OpenStudio-PAT) GitHub page.

# Where to Find OpenStudio SDK Documentation

- OpenStudio SDK release documentation, including these release notes, tutorials, and other user documentation, is available at [https://www.openstudio.net/](https://www.openstudio.net/.)
- C++ API documentation is available at [OpenStudio SDK Documentation](https://openstudio-sdk-documentation.s3.amazonaws.com/index.html)
- Measure development documentation is available at [OpenStudio Measure Writer's Reference Guide](http://nrel.github.io/OpenStudio-user-documentation/reference/measure_writing_guide/ )
- A roadmap for planned features is available at [Roadmap](http://nrel.github.io/OpenStudio-user-documentation/getting_started/roadmap/)

# Installation Notes

OpenStudio SDK 3.10.0 is supported on:

* 64-bit Windows 7 – 11
* macOS: 11.6+ x86_64, 12.1+ arm64
* Ubuntu: 22.04 x86_64, 24.04 x86_64, 22.04 arm64

OpenStudio SDK 3.10.0 supports [EnergyPlus Release 25.1.0, Bug Fix Edition](https://github.com/NREL/EnergyPlus/releases/tag/v25.1.0-WithDSOASpaceListFixes), which is bundled with the OpenStudio installer. It is no longer necessary to download and install EnergyPlus separately. Other builds of EnergyPlus are not supported by OpenStudio SDK 3.10.0.

OpenStudio SDK 3.10.0 supports Radiance 5.0.a.12, which is bundled with the OpenStudio installer; users no longer must install Radiance separately, and OpenStudio will use the included Radiance version regardless of any other versions that may be installed on the system. Other builds of Radiance are not supported by OpenStudio SDK 3.10.0.

As usual, you can refer to the **[OpenStudio SDK Compatibility Matrix](https://github.com/NREL/OpenStudio/wiki/OpenStudio-SDK-Version-Compatibility-Matrix)** for more information.


## Installation Steps

- Download and install [OpenStudio SDK](https://github.com/NREL/openstudio) and/or any third party tool that embeds the OpenStudio SDK into their software such as [openstudiocoalition/OpenStudioApplication](https://github.com/openstudiocoalition/OpenStudioApplication) depending on your needs. Select components for installation. Note that OpenStudio Application is a standalone app and does not require you to install OpenStudio SDK.
- A BCL AUth Key is no longer needed to access content on [Building Component Library (BCL)](https://bcl.nrel.gov) BCL is typically accessed through third party OpenStudio applications to provide modelers access to building components and OpenStudio measures; however you an also browse it and download content using the BCL link above.

For help with common installation problems please visit [Getting Started](http://nrel.github.io/OpenStudio-user-documentation/getting_started/getting_started/).

# OpenStudio SDK: Changelog

The 3.10.0 is a **major** release. This update includes several new features, performance improvements, and bug fixes.

## C++ Workflow code

As of OpenStudio SDK 3.7.0 a re-written workflow written in C++ is used by default in place of the Ruby based Workflow Gem that had been used in the past. This enhancement is in support of Python measures being used in an OpenStudio workflow, including mixed language workflows that include both Ruby Measures and Python Measures. If you need to use the older Workflow Gem implementation, you can do that by using the `classic` subcommand after `openstudio`. `classic` will be deprecated in a future version of OpenStudio.

## Python Bindings

As of OpenStudio SDK 3.2.0, Python bindings are officially supported and distributed through Python Package Index (PyPI). To install, users will need to have Python3 installed along with pip and simply run the following command in a terminal window.

`pip install openstudio==3.10.0`

Please see [openstudio on PyPi](https://pypi.org/project/openstudio/) for further instructions on how to install. Users can also visit the test channel at [openstudio on TestPyPi](https://test.pypi.org/project/openstudio/) to install development bindings.

You can also refer to the [OpenStudio SDK Python Binding Version Compatibility Matrix](https://github.com/NREL/OpenStudio/wiki/OpenStudio-SDK-Python-Binding-Version-Compatibility-Matrix) to see the list of supported platforms and python versions.

## New Features, Major Fixes and API-breaking changes

* [#5326](https://github.com/NREL/OpenStudio/pull/5326) - Wrap `ZoneHVAC:EvaporativeCoolerUnit`
    * The object was wrapped in the SDK.
    * Note: in EnergyPlus 24.2.0, the `Zone Relief Air Node Name` is an optional field. The OpenStudio SDK always fills with the connected zone's Exhaust Air Node, meaning the airflow is always being balanced by EnergyPlus: the object will extract air from the zone to balance the air supplied to the zone by the cooler outlet node.

* [#5369](https://github.com/NREL/OpenStudio/pull/5369) - Wrap `Output:Table:Annual` and `Output:Table:Monthly`
    * Both objects were wrapped in the SDK
    * The OutputTableMonthly also includes a convenient factory methods to create the reports that are in the E+ datasets/StandardReports.idf
        * See `std::vector<std::string> OutputTableMonthly::validStandardReportNames` to get a list of valid methods
        * and the factory method itself: `OutputTableMonthly OutputTableMonthly::fromStandardReports(const Model& model, const std::string& standardReportName)`

* [#5365](https://github.com/NREL/OpenStudio/pull/5365) - E+ 25.1.0: Wrap `OutputControl:ResilienceSummaries`

* [#5312](https://github.com/NREL/OpenStudio/pull/5312) - Wrap `PythonPlugin:SearchPaths`
    * The unique object was wrapped in the SDK.
    * Forward translation intentionally happens before PythonPlugin_Instance so that there cannot be two PythonPlugin_SearchPaths objects.

* [#5134](https://github.com/NREL/OpenStudio/pull/5134) - Addresses #5132, EPW design condition methods should return boost::optional doubles or integers
    * `EpwDesignCondition` has many API-breaking changes related to its getters. The previous behavior was to misleadingly return a value of 0 for any empty design condition header field. The types for the getters are now either boost::optional<double> or boost::optional<int>.

* [#5350](https://github.com/NREL/OpenStudio/pull/5350) - Wrap `CoilSystem:Cooling:Water`
    * The object was wrapped in the SDK.
    * This coil system enables "Water Side Economizer Mode" and "Wrap Around Water Coil Heat Recovery Mode".

* [#5426](https://github.com/NREL/OpenStudio/pull/5426) - Re-wrap Thermochromic window model properly to handle extensible fields and translation to/from EnergyPlus
    * While the class `ThermochromicGlazing` has been in the model namespace for a long time, it was actually not properly wrapped, unsable, and not forward translated

* [#5384](https://github.com/NREL/OpenStudio/pull/5384) + [#5403](https://github.com/NREL/OpenStudio/pull/5403)- OutdoorAir upgrades
    * Do **not** add a `Controller:MechanicalVentilation` if it does not have a `DesignSpecification:OutdoorAir` on it, which would produce an E+ `SevereError`
    * Use a `DesignSpecification:OutdoorAir:SpaceList` if appropriate instead of using the first `DesignSpecification:OutdoorAir` found
        * In case you have several Spaces with unique DSOAs that are using an absolute value (Outdoor Air Flow Rate) in the same ThermalZone, this previously would lead to an incorrect amount of Outdoor Air

* [#5367](https://github.com/NREL/OpenStudio/pull/5367) - Add a new `ReportingMeasure::modelOutputRequests(model, runner, argument_map)` that runs before E+ FT

* [#5385](https://github.com/NREL/OpenStudio/pull/5385) - add `WorkflowJSON::setRootDir(path&)` and `setRunDir(path&)`

* [#5394](https://github.com/NREL/OpenStudio/pull/5394) - `--bundle` options not working for CLI in docker-openstudio, possibly OS itself

## Minor changes and bug fixes

* [#5396](https://github.com/NREL/OpenStudio/pull/5396) - Create convenience method to set SpaceInfiltrationDesignFlowRate values from Space

* [#5372](https://github.com/NREL/OpenStudio/pull/5372) - add FuelTypes to PlantComponentUserDefined

* [#5401](https://github.com/NREL/OpenStudio/pull/5402) - BCL measure update picks up subfolders like `resources/__pycache__` or `tests/.pytest_cache`

* [#5304](https://github.com/NREL/OpenStudio/pull/5304) - Measure manager fixups and improvements

* [#5378](https://github.com/NREL/OpenStudio/pull/5378) - Old materials OSC (< 0.7.4) cannot be loaded anymore

* [#5373](https://github.com/NREL/OpenStudio/pull/5373) - ThreeJSForwardTranslator adds unnecessary RenderingColor objects for AirLoopHVAC

* [#5382](https://github.com/NREL/OpenStudio/pull/5382) - Add a helper `IdfObject::initializeFields(bool fill_default)`

* [#5334](https://github.com/NREL/OpenStudio/pull/5334) - Register AFN scheduleTypeRegistry.

* [#5345](https://github.com/NREL/OpenStudio/pull/5345) - Design Range Temperature for OS:CoolingTower:SingleSpeed is not being converted into EnergyPlus

* [#5322](https://github.com/NREL/OpenStudio/pull/5322) - AirLoopHVACUnitaryHeatPump(Multispeed) fixes

* [#5333](https://github.com/NREL/OpenStudio/pull/5333) - Typo in Schematron extension: .sch, not .sct!

* [#5337](https://github.com/NREL/OpenStudio/pull/5337) - Fixes `UnitarySystemPerformance:Multispeed` number of cooling speeds w/ `Coil:Cooling:DX`

* [#5325](https://github.com/NREL/OpenStudio/pull/5325) - Wrap GroundHeatExchangerVertical "Depth of Top of Borehole" field

* [#5316](https://github.com/NREL/OpenStudio/pull/5316) - OpenStudio CLI unable to require uuid in OpenStudio 3.9 (thereby breaking dependencies like openstudio-extension)

* [#5320](https://github.com/NREL/OpenStudio/pull/5320) - gbXML Reverse Translator - Scan for the gbxml Schema version: skip schema validation with a warning when not 7.03

* [#5305](https://github.com/NREL/OpenStudio/pull/5305) - Remove SpaceAndSpaceGroupNames from Building

* [#5321](https://github.com/NREL/OpenStudio/pull/5321) - Add Tank Element Control Logic field to WaterHeaterHeatPump

* [#5374](https://github.com/NREL/OpenStudio/pull/5374) - DefrostEnergyInputRatioModifierFunctionofTemperatureCurve is orphaned when AirConditionerVariableRefrigerantFlow is removed

* [#5413](https://github.com/NREL/OpenStudio/pull/5413) - Deal with `ASHRAETau2017`

* [#5422](https://github.com/NREL/OpenStudio/pull/5422) - define SWIG_PYTHON_SILENT_MEMLEAK to shush the python destructor warnings


Refer to the changelog on the release page at [v3.10.0](https://github.com/NREL/OpenStudio/releases/v3.10.0)

**Full Changelog**: https://github.com/NREL/OpenStudio/compare/v3.9.0...v3.10.0

**New Contributors**:

### OpenStudio Standards v0.8.2

Update the openstudio-standards gem to version [0.8.2](https://github.com/NREL/openstudio-standards/releases/tag/v0.8.2)
In addition to some refactoring, this release also included conversion of 90.1 data to formal database.

---
# This YAML header controls the pandoc (via TeX) to PDF settings
# To convert the markdown to pdf, do `pandoc release_notes.md -o release_notes.pdf`
title:  'OpenStudio Release Notes - 3.10.0'
author:
- National Renewable Energy Laboratory
colorlinks: true
linkcolor: blue
urlcolor: Mahogany
toccolor: gray
geometry:
- margin=1in
---
