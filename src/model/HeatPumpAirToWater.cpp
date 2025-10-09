/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include "HeatPumpAirToWater.hpp"
#include "HeatPumpAirToWater_Impl.hpp"

#include "Model.hpp"
#include "Curve.hpp"
#include "Curve_Impl.hpp"
#include "HeatPumpAirToWaterHeating.hpp"
#include "HeatPumpAirToWaterHeating_Impl.hpp"
#include "HeatPumpAirToWaterCooling.hpp"
#include "HeatPumpAirToWaterCooling_Impl.hpp"
#include "PlantLoop.hpp"
#include "Schedule.hpp"
#include "Schedule_Impl.hpp"
#include "ScheduleTypeLimits.hpp"
#include "ScheduleTypeRegistry.hpp"

#include "../utilities/core/Assert.hpp"
#include "../utilities/core/ContainersMove.hpp"
#include "../utilities/data/DataEnums.hpp"

#include <utilities/idd/IddFactory.hxx>
#include <utilities/idd/IddEnums.hxx>
#include <utilities/idd/OS_HeatPump_AirToWater_FieldEnums.hxx>

namespace openstudio {
namespace model {

  namespace detail {

    HeatPumpAirToWater_Impl::HeatPumpAirToWater_Impl(const IdfObject& idfObject, Model_Impl* model, bool keepHandle)
      : StraightComponent_Impl(idfObject, model, keepHandle) {
      OS_ASSERT(idfObject.iddObject().type() == HeatPumpAirToWater::iddObjectType());
    }

    HeatPumpAirToWater_Impl::HeatPumpAirToWater_Impl(const openstudio::detail::WorkspaceObject_Impl& other, Model_Impl* model, bool keepHandle)
      : StraightComponent_Impl(other, model, keepHandle) {
      OS_ASSERT(other.iddObject().type() == HeatPumpAirToWater::iddObjectType());
    }

    HeatPumpAirToWater_Impl::HeatPumpAirToWater_Impl(const HeatPumpAirToWater_Impl& other, Model_Impl* model, bool keepHandle)
      : StraightComponent_Impl(other, model, keepHandle) {}

    const std::vector<std::string>& HeatPumpAirToWater_Impl::outputVariableNames() const {
      static const std::vector<std::string> result{
        "Heat Pump Part Load Ratio in Heating Mode",
        "Heat Pump Cycling Ratio in Heating Mode",
        "Heat Pump Load Side Heat Transfer Rate in Heating Mode",
        "Heat Pump Load Side Heat Transfer Energy in Heating Mode",
        "Heat Pump Source Side Heat Transfer Rate in Heating Mode",
        "Heat Pump Source Side Heat Transfer Energy in Heating Mode",
        "Heat Pump Load Side Inlet Temperature in Heating Mode",
        "Heat Pump Load Side Outlet Temperature in Heating Mode",
        "Heat Pump Source Side Inlet Temperature in Heating Mode",
        "Heat Pump Source Side Outlet Temperature in Heating Mode",
        "Heat Pump Electricity Rate in Heating Mode",
        "Heat Pump Electricity Energy in Heating Mode",
        "Heat Pump Load Due To Defrost",
        "Heat Pump Fractional Defrost Time",
        "Heat Pump Defrost Electricity Rate",
        "Heat Pump Defrost Electricity Energy",
        "Heat Pump Load Side Mass Flow Rate in Heating Mode",
        "Heat Pump Source Side Mass Flow Rate in Heating Mode",
        "Heat Pump Total Heating Rate",
        "Heat Pump Number Of Heating Unit On",
        "Heat Pump Speed Level in Heating Mode",
        "Heat Pump Speed Ratio in Heating Mode",
        "Heat Pump Air Flow Rate in Heating Mode",
        "Heat Pump Inlet Air Temperature in Heating Mode",
        "Heat Pump Outlet Air Temperature in Heating Mode",
        "Heat Pump Capacity Temperature Modifier in Heating Mode",
        "Heat Pump EIR Temperature Modifier in Heating Mode",
        "Heat Pump EIR PLR Modifier in Heating Mode",
        "Heat Pump Crankcase Heater Electricity Rate",
        "Heat Pump Crankcase Heater Electricity Energy",
        "Heat Pump Heating COP",
        "Heat Pump Part Load Ratio in Cooling Mode",
        "Heat Pump Cycling Ratio in Cooling Mode",
        "Heat Pump Load Side Heat Transfer Rate in Cooling Mode",
        "Heat Pump Load Side Heat Transfer Energy in Cooling Mode",
        "Heat Pump Source Side Heat Transfer Rate in Cooling Mode",
        "Heat Pump Source Side Heat Transfer Energy in Cooling Mode",
        "Heat Pump Load Side Inlet Temperature in Cooling Mode",
        "Heat Pump Load Side Outlet Temperature in Cooling Mode",
        "Heat Pump Source Side Inlet Temperature in Cooling Mode",
        "Heat Pump Source Side Outlet Temperature in Cooling Mode",
        "Heat Pump Electricity Rate in Cooling Mode",
        "Heat Pump Electricity Energy in Cooling Mode",
        "Heat Pump Load Side Mass Flow Rate in Cooling Mode",
        "Heat Pump Source Side Mass Flow Rate in Cooling Mode",
        "Heat Pump Total Cooling Rate",
        "Heat Pump Number Of Cooling Unit On",
        "Heat Pump Speed Level in Cooling Mode",
        "Heat Pump Speed Ratio in Cooling Mode",
        "Heat Pump Air Flow Rate in Cooling Mode",
        "Heat Pump Inlet Air Temperature in Cooling Mode",
        "Heat Pump Outlet Air Temperature in Cooling Mode",
        "Heat Pump Capacity Temperature Modifier in Cooling Mode",
        "Heat Pump EIR Temperature Modifier in Cooling Mode",
        "Heat Pump EIR PLR Modifier in Cooling Mode",
        "Heat Pump Cooling COP",
      };
      return result;
    }

    IddObjectType HeatPumpAirToWater_Impl::iddObjectType() const {
      return HeatPumpAirToWater::iddObjectType();
    }

    std::vector<ScheduleTypeKey> HeatPumpAirToWater_Impl::getScheduleTypeKeys(const Schedule& schedule) const {
      std::vector<ScheduleTypeKey> result;
      const UnsignedVector fieldIndices = getSourceIndices(schedule.handle());
      if (std::find(fieldIndices.cbegin(), fieldIndices.cend(), OS_HeatPump_AirToWaterFields::OperatingModeControlScheduleName)
          != fieldIndices.cend()) {
        result.emplace_back("HeatPumpAirToWater", "Operating Mode Control");
      }
      return result;
    }

    unsigned HeatPumpAirToWater_Impl::inletPort() const {
      return OS_HeatPump_AirToWaterFields::AirInletNodeName;
    }

    unsigned HeatPumpAirToWater_Impl::outletPort() const {
      return OS_HeatPump_AirToWaterFields::AirOutletNodeName;
    }

    bool HeatPumpAirToWater_Impl::addToNode(Node& /*node*/) {
      LOG(Info, "Use the underlying HeatPumpAirToWaterCooling / HeatPumpAirToWaterHeating objects to add to a plant loop.");
      return false;
    }

    std::vector<ModelObject> HeatPumpAirToWater_Impl::children() const {
      std::vector<ModelObject> children;
      if (auto c_ = defrostEnergyInputRatioFunctionofTemperatureCurve()) {
        children.emplace_back(std::move(*c_));
      }
      if (auto c_ = crankcaseHeaterCapacityFunctionofTemperatureCurve()) {
        children.emplace_back(std::move(*c_));
      }
      if (auto c_ = coolingOperationMode()) {
        children.emplace_back(std::move(*c_));
      }
      if (auto c_ = heatingOperationMode()) {
        children.emplace_back(std::move(*c_));
      }
      return children;
    }

    ModelObject HeatPumpAirToWater_Impl::clone(Model model) const {
      auto t_clone = StraightComponent_Impl::clone(model).cast<HeatPumpAirToWater>();
      // We clone the operation modes, because the autosizing is reported at the wrapper level
      if (auto coolingOpMode_ = coolingOperationMode()) {
        auto coolingOpModeClone = coolingOpMode_->clone(model).cast<HeatPumpAirToWaterCooling>();
        bool ok = t_clone.setCoolingOperationMode(coolingOpModeClone);
        OS_ASSERT(ok);
      }
      if (auto heatingOpMode_ = heatingOperationMode()) {
        auto heatingOpModeClone = heatingOpMode_->clone(model).cast<HeatPumpAirToWaterHeating>();
        bool ok = t_clone.setHeatingOperationMode(heatingOpModeClone);
        OS_ASSERT(ok);
      }

      return std::move(t_clone);
    }

    std::vector<IdfObject> HeatPumpAirToWater_Impl::remove() {
      std::vector<IdfObject> result;

      // Don't delete the underlying HeatPumpAirToWaterCooling / Heating objects if used by several HeatPumpAirToWater
      if (auto coolingOpMode_ = coolingOperationMode()) {
        if (coolingOpMode_->heatPumpAirToWaters().size() > 1) {
          resetCoolingOperationMode();
        } else {
          // If we are the only one using this cooling operation mode, remove it (it will remove the children too)
          resetCoolingOperationMode();  // Need to reset so it's marked as removable first
          result = coolingOpMode_->remove();
        }
      }
      if (auto heatingOpMode_ = heatingOperationMode()) {
        if (heatingOpMode_->heatPumpAirToWaters().size() > 1) {
          resetHeatingOperationMode();
        } else {
          // If we are the only one using this heating operation mode, remove it (it will remove the children too)
          resetHeatingOperationMode();
          openstudio::detail::concat_helper(result, heatingOpMode_->remove());
        }
      }
      openstudio::detail::concat_helper(result, StraightComponent_Impl::remove());
      return result;
    }

    ComponentType HeatPumpAirToWater_Impl::componentType() const {
      bool has_cooling = coolingOperationMode().is_initialized();
      bool has_heating = heatingOperationMode().is_initialized();
      if (has_cooling && has_heating) {
        return ComponentType::Both;
      } else if (has_cooling) {
        return ComponentType::Cooling;
      } else if (has_heating) {
        return ComponentType::Heating;
      }
      return ComponentType::None;
    }

    std::vector<FuelType> HeatPumpAirToWater_Impl::coolingFuelTypes() const {
      if (auto cc_ = coolingOperationMode()) {
        return cc_->coolingFuelTypes();
      }
      return {};
    }

    std::vector<FuelType> HeatPumpAirToWater_Impl::heatingFuelTypes() const {
      if (auto hc_ = heatingOperationMode()) {
        return hc_->heatingFuelTypes();
      }
      return {};
    }

    std::vector<AppGFuelType> HeatPumpAirToWater_Impl::appGHeatingFuelTypes() const {
      if (auto hc_ = heatingOperationMode()) {
        return hc_->appGHeatingFuelTypes();
      }
      return {};
    }

    std::string HeatPumpAirToWater_Impl::operatingModeControlMethod() const {
      boost::optional<std::string> value = getString(OS_HeatPump_AirToWaterFields::OperatingModeControlMethod, true);
      OS_ASSERT(value);
      return value.get();
    }

    std::string HeatPumpAirToWater_Impl::operatingModeControlOptionforMultipleUnit() const {
      boost::optional<std::string> value = getString(OS_HeatPump_AirToWaterFields::OperatingModeControlOptionforMultipleUnit, true);
      OS_ASSERT(value);
      return value.get();
    }

    boost::optional<Schedule> HeatPumpAirToWater_Impl::operatingModeControlSchedule() const {
      return getObject<ModelObject>().getModelObjectTarget<Schedule>(OS_HeatPump_AirToWaterFields::OperatingModeControlScheduleName);
    }

    double HeatPumpAirToWater_Impl::minimumPartLoadRatio() const {
      boost::optional<double> value = getDouble(OS_HeatPump_AirToWaterFields::MinimumPartLoadRatio, true);
      OS_ASSERT(value);
      return value.get();
    }

    boost::optional<std::string> HeatPumpAirToWater_Impl::airInletNodeName() const {
      return getString(OS_HeatPump_AirToWaterFields::AirInletNodeName, false, true);  // Return empty if not initialized
    }

    boost::optional<std::string> HeatPumpAirToWater_Impl::airOutletNodeName() const {
      return getString(OS_HeatPump_AirToWaterFields::AirOutletNodeName, false, true);  // Return empty if not initialized
    }

    double HeatPumpAirToWater_Impl::maximumOutdoorDryBulbTemperatureForDefrostOperation() const {
      boost::optional<double> value = getDouble(OS_HeatPump_AirToWaterFields::MaximumOutdoorDryBulbTemperatureForDefrostOperation, true);
      OS_ASSERT(value);
      return value.get();
    }

    std::string HeatPumpAirToWater_Impl::heatPumpDefrostControl() const {
      boost::optional<std::string> value = getString(OS_HeatPump_AirToWaterFields::HeatPumpDefrostControl, true);
      OS_ASSERT(value);
      return value.get();
    }

    double HeatPumpAirToWater_Impl::heatPumpDefrostTimePeriodFraction() const {
      boost::optional<double> value = getDouble(OS_HeatPump_AirToWaterFields::HeatPumpDefrostTimePeriodFraction, true);
      OS_ASSERT(value);
      return value.get();
    }

    boost::optional<double> HeatPumpAirToWater_Impl::resistiveDefrostHeaterCapacity() const {
      return getDouble(OS_HeatPump_AirToWaterFields::ResistiveDefrostHeaterCapacity, true);
    }

    bool HeatPumpAirToWater_Impl::isResistiveDefrostHeaterCapacityAutosized() const {
      bool result = false;
      boost::optional<std::string> value = getString(OS_HeatPump_AirToWaterFields::ResistiveDefrostHeaterCapacity, true);
      if (value) {
        result = openstudio::istringEqual(value.get(), "autosize");
      }
      return result;
    }

    boost::optional<double> HeatPumpAirToWater_Impl::autosizedResistiveDefrostHeaterCapacity() const {
      return getAutosizedValue("TODO_CHECK_SQL Resistive Defrost Heater Capacity", "W");
    }

    boost::optional<Curve> HeatPumpAirToWater_Impl::defrostEnergyInputRatioFunctionofTemperatureCurve() const {
      return getObject<ModelObject>().getModelObjectTarget<Curve>(
        OS_HeatPump_AirToWaterFields::DefrostEnergyInputRatioFunctionofTemperatureCurveName);
    }

    int HeatPumpAirToWater_Impl::heatPumpMultiplier() const {
      boost::optional<int> value = getInt(OS_HeatPump_AirToWaterFields::HeatPumpMultiplier, true);
      OS_ASSERT(value);
      return value.get();
    }

    std::string HeatPumpAirToWater_Impl::controlType() const {
      boost::optional<std::string> value = getString(OS_HeatPump_AirToWaterFields::ControlType, true);
      OS_ASSERT(value);
      return value.get();
    }

    double HeatPumpAirToWater_Impl::crankcaseHeaterCapacity() const {
      boost::optional<double> value = getDouble(OS_HeatPump_AirToWaterFields::CrankcaseHeaterCapacity, true);
      OS_ASSERT(value);
      return value.get();
    }

    boost::optional<Curve> HeatPumpAirToWater_Impl::crankcaseHeaterCapacityFunctionofTemperatureCurve() const {
      return getObject<ModelObject>().getModelObjectTarget<Curve>(
        OS_HeatPump_AirToWaterFields::CrankcaseHeaterCapacityFunctionofTemperatureCurveName);
    }

    double HeatPumpAirToWater_Impl::maximumAmbientTemperatureforCrankcaseHeaterOperation() const {
      boost::optional<double> value = getDouble(OS_HeatPump_AirToWaterFields::MaximumAmbientTemperatureforCrankcaseHeaterOperation, true);
      OS_ASSERT(value);
      return value.get();
    }

    boost::optional<HeatPumpAirToWaterHeating> HeatPumpAirToWater_Impl::heatingOperationMode() const {
      return getObject<ModelObject>().getModelObjectTarget<HeatPumpAirToWaterHeating>(OS_HeatPump_AirToWaterFields::HeatingOperationMode);
    }

    boost::optional<HeatPumpAirToWaterCooling> HeatPumpAirToWater_Impl::coolingOperationMode() const {
      return getObject<ModelObject>().getModelObjectTarget<HeatPumpAirToWaterCooling>(OS_HeatPump_AirToWaterFields::CoolingOperationMode);
    }

    bool HeatPumpAirToWater_Impl::setOperatingModeControlMethod(const std::string& operatingModeControlMethod) {
      const bool result = setString(OS_HeatPump_AirToWaterFields::OperatingModeControlMethod, operatingModeControlMethod);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setOperatingModeControlOptionforMultipleUnit(const std::string& operatingModeControlOptionforMultipleUnit) {
      const bool result =
        setString(OS_HeatPump_AirToWaterFields::OperatingModeControlOptionforMultipleUnit, operatingModeControlOptionforMultipleUnit);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setOperatingModeControlSchedule(Schedule& operatingModeControlSchedule) {
      const bool result = setSchedule(OS_HeatPump_AirToWaterFields::OperatingModeControlScheduleName, "HeatPumpAirToWater", "Operating Mode Control",
                                      operatingModeControlSchedule);
      return result;
    }

    void HeatPumpAirToWater_Impl::resetOperatingModeControlSchedule() {
      const bool result = setString(OS_HeatPump_AirToWaterFields::OperatingModeControlScheduleName, "");
      OS_ASSERT(result);
    }

    bool HeatPumpAirToWater_Impl::setMinimumPartLoadRatio(double minimumPartLoadRatio) {
      const bool result = setDouble(OS_HeatPump_AirToWaterFields::MinimumPartLoadRatio, minimumPartLoadRatio);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setAirInletNodeName(const std::string& airInletNodeName) {
      const bool result = setString(OS_HeatPump_AirToWaterFields::AirInletNodeName, airInletNodeName);
      OS_ASSERT(result);
      return result;
    }

    void HeatPumpAirToWater_Impl::resetAirInletNodeName() {
      const bool result = setString(OS_HeatPump_AirToWaterFields::AirInletNodeName, "");
      OS_ASSERT(result);
    }

    bool HeatPumpAirToWater_Impl::setAirOutletNodeName(const std::string& airOutletNodeName) {
      const bool result = setString(OS_HeatPump_AirToWaterFields::AirOutletNodeName, airOutletNodeName);
      OS_ASSERT(result);
      return result;
    }

    void HeatPumpAirToWater_Impl::resetAirOutletNodeName() {
      const bool result = setString(OS_HeatPump_AirToWaterFields::AirOutletNodeName, "");
      OS_ASSERT(result);
    }

    bool HeatPumpAirToWater_Impl::setMaximumOutdoorDryBulbTemperatureForDefrostOperation(double maximumOutdoorDryBulbTemperatureForDefrostOperation) {
      const bool result = setDouble(OS_HeatPump_AirToWaterFields::MaximumOutdoorDryBulbTemperatureForDefrostOperation,
                                    maximumOutdoorDryBulbTemperatureForDefrostOperation);
      OS_ASSERT(result);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setHeatPumpDefrostControl(const std::string& heatPumpDefrostControl) {
      const bool result = setString(OS_HeatPump_AirToWaterFields::HeatPumpDefrostControl, heatPumpDefrostControl);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setHeatPumpDefrostTimePeriodFraction(double heatPumpDefrostTimePeriodFraction) {
      const bool result = setDouble(OS_HeatPump_AirToWaterFields::HeatPumpDefrostTimePeriodFraction, heatPumpDefrostTimePeriodFraction);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setResistiveDefrostHeaterCapacity(double resistiveDefrostHeaterCapacity) {
      const bool result = setDouble(OS_HeatPump_AirToWaterFields::ResistiveDefrostHeaterCapacity, resistiveDefrostHeaterCapacity);
      return result;
    }

    void HeatPumpAirToWater_Impl::autosizeResistiveDefrostHeaterCapacity() {
      const bool result = setString(OS_HeatPump_AirToWaterFields::ResistiveDefrostHeaterCapacity, "autosize");
      OS_ASSERT(result);
    }

    bool
      HeatPumpAirToWater_Impl::setDefrostEnergyInputRatioFunctionofTemperatureCurve(const Curve& defrostEnergyInputRatioFunctionofTemperatureCurve) {
      const bool result = setPointer(OS_HeatPump_AirToWaterFields::DefrostEnergyInputRatioFunctionofTemperatureCurveName,
                                     defrostEnergyInputRatioFunctionofTemperatureCurve.handle());
      return result;
    }

    void HeatPumpAirToWater_Impl::resetDefrostEnergyInputRatioFunctionofTemperatureCurve() {
      const bool result = setString(OS_HeatPump_AirToWaterFields::DefrostEnergyInputRatioFunctionofTemperatureCurveName, "");
      OS_ASSERT(result);
    }

    bool HeatPumpAirToWater_Impl::setHeatPumpMultiplier(int heatPumpMultiplier) {
      const bool result = setInt(OS_HeatPump_AirToWaterFields::HeatPumpMultiplier, heatPumpMultiplier);
      OS_ASSERT(result);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setControlType(const std::string& controlType) {
      const bool result = setString(OS_HeatPump_AirToWaterFields::ControlType, controlType);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setCrankcaseHeaterCapacity(double crankcaseHeaterCapacity) {
      const bool result = setDouble(OS_HeatPump_AirToWaterFields::CrankcaseHeaterCapacity, crankcaseHeaterCapacity);
      return result;
    }

    bool
      HeatPumpAirToWater_Impl::setCrankcaseHeaterCapacityFunctionofTemperatureCurve(const Curve& crankcaseHeaterCapacityFunctionofTemperatureCurve) {
      const bool result = setPointer(OS_HeatPump_AirToWaterFields::CrankcaseHeaterCapacityFunctionofTemperatureCurveName,
                                     crankcaseHeaterCapacityFunctionofTemperatureCurve.handle());
      return result;
    }

    void HeatPumpAirToWater_Impl::resetCrankcaseHeaterCapacityFunctionofTemperatureCurve() {
      const bool result = setString(OS_HeatPump_AirToWaterFields::CrankcaseHeaterCapacityFunctionofTemperatureCurveName, "");
      OS_ASSERT(result);
    }

    bool
      HeatPumpAirToWater_Impl::setMaximumAmbientTemperatureforCrankcaseHeaterOperation(double maximumAmbientTemperatureforCrankcaseHeaterOperation) {
      const bool result = setDouble(OS_HeatPump_AirToWaterFields::MaximumAmbientTemperatureforCrankcaseHeaterOperation,
                                    maximumAmbientTemperatureforCrankcaseHeaterOperation);
      return result;
    }

    bool HeatPumpAirToWater_Impl::setHeatingOperationMode(const HeatPumpAirToWaterHeating& heatingOperationMode) {
      const bool result = setPointer(OS_HeatPump_AirToWaterFields::HeatingOperationMode, heatingOperationMode.handle());
      return result;
    }

    void HeatPumpAirToWater_Impl::resetHeatingOperationMode() {
      const bool result = setString(OS_HeatPump_AirToWaterFields::HeatingOperationMode, "");
      OS_ASSERT(result);
    }

    bool HeatPumpAirToWater_Impl::setCoolingOperationMode(const HeatPumpAirToWaterCooling& coolingOperationMode) {
      const bool result = setPointer(OS_HeatPump_AirToWaterFields::CoolingOperationMode, coolingOperationMode.handle());
      return result;
    }

    void HeatPumpAirToWater_Impl::resetCoolingOperationMode() {
      const bool result = setString(OS_HeatPump_AirToWaterFields::CoolingOperationMode, "");
      OS_ASSERT(result);
    }

    void HeatPumpAirToWater_Impl::autosize() {
      autosizeResistiveDefrostHeaterCapacity();
    }

    void HeatPumpAirToWater_Impl::applySizingValues() {
      if (boost::optional<double> val_ = autosizedResistiveDefrostHeaterCapacity()) {
        setResistiveDefrostHeaterCapacity(*val_);
      }
    }

    boost::optional<PlantLoop> HeatPumpAirToWater_Impl::coolingLoop() const {
      if (auto mode_ = coolingOperationMode()) {
        return mode_->plantLoop();
      }
      return boost::none;
    }

    boost::optional<PlantLoop> HeatPumpAirToWater_Impl::heatingLoop() const {
      if (auto mode_ = heatingOperationMode()) {
        return mode_->plantLoop();
      }
      return boost::none;
    }

  }  // namespace detail

  HeatPumpAirToWater::HeatPumpAirToWater(const Model& model) : StraightComponent(HeatPumpAirToWater::iddObjectType(), model) {
    OS_ASSERT(getImpl<detail::HeatPumpAirToWater_Impl>());

    // IDD defaults
    bool ok = true;
    ok &= setOperatingModeControlMethod("Load");
    ok &= setOperatingModeControlOptionforMultipleUnit("SingleMode");
    ok &= setMinimumPartLoadRatio(0.0);
    ok &= setMaximumOutdoorDryBulbTemperatureForDefrostOperation(10.0);
    ok &= setHeatPumpDefrostControl("None");
    ok &= setHeatPumpDefrostTimePeriodFraction(0.058333);
    ok &= setResistiveDefrostHeaterCapacity(0.0);
    ok &= setHeatPumpMultiplier(1);
    ok &= setControlType("VariableSpeed");
    ok &= setCrankcaseHeaterCapacity(0.0);
    ok &= setMaximumAmbientTemperatureforCrankcaseHeaterOperation(10.0);
    OS_ASSERT(ok);
  }

  IddObjectType HeatPumpAirToWater::iddObjectType() {
    return {IddObjectType::OS_HeatPump_AirToWater};
  }

  std::vector<std::string> HeatPumpAirToWater::operatingModeControlMethodValues() {
    return getIddKeyNames(IddFactory::instance().getObject(iddObjectType()).get(), OS_HeatPump_AirToWaterFields::OperatingModeControlMethod);
  }

  std::vector<std::string> HeatPumpAirToWater::operatingModeControlOptionforMultipleUnitValues() {
    return getIddKeyNames(IddFactory::instance().getObject(iddObjectType()).get(),
                          OS_HeatPump_AirToWaterFields::OperatingModeControlOptionforMultipleUnit);
  }

  std::vector<std::string> HeatPumpAirToWater::heatPumpDefrostControlValues() {
    return getIddKeyNames(IddFactory::instance().getObject(iddObjectType()).get(), OS_HeatPump_AirToWaterFields::HeatPumpDefrostControl);
  }

  std::vector<std::string> HeatPumpAirToWater::controlTypeValues() {
    return getIddKeyNames(IddFactory::instance().getObject(iddObjectType()).get(), OS_HeatPump_AirToWaterFields::ControlType);
  }

  std::string HeatPumpAirToWater::operatingModeControlMethod() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->operatingModeControlMethod();
  }

  std::string HeatPumpAirToWater::operatingModeControlOptionforMultipleUnit() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->operatingModeControlOptionforMultipleUnit();
  }

  boost::optional<Schedule> HeatPumpAirToWater::operatingModeControlSchedule() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->operatingModeControlSchedule();
  }

  double HeatPumpAirToWater::minimumPartLoadRatio() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->minimumPartLoadRatio();
  }

  boost::optional<std::string> HeatPumpAirToWater::airInletNodeName() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->airInletNodeName();
  }

  boost::optional<std::string> HeatPumpAirToWater::airOutletNodeName() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->airOutletNodeName();
  }

  double HeatPumpAirToWater::maximumOutdoorDryBulbTemperatureForDefrostOperation() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->maximumOutdoorDryBulbTemperatureForDefrostOperation();
  }

  std::string HeatPumpAirToWater::heatPumpDefrostControl() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->heatPumpDefrostControl();
  }

  double HeatPumpAirToWater::heatPumpDefrostTimePeriodFraction() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->heatPumpDefrostTimePeriodFraction();
  }

  boost::optional<double> HeatPumpAirToWater::resistiveDefrostHeaterCapacity() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->resistiveDefrostHeaterCapacity();
  }

  bool HeatPumpAirToWater::isResistiveDefrostHeaterCapacityAutosized() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->isResistiveDefrostHeaterCapacityAutosized();
  }

  boost::optional<double> HeatPumpAirToWater::autosizedResistiveDefrostHeaterCapacity() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->autosizedResistiveDefrostHeaterCapacity();
  }

  boost::optional<Curve> HeatPumpAirToWater::defrostEnergyInputRatioFunctionofTemperatureCurve() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->defrostEnergyInputRatioFunctionofTemperatureCurve();
  }

  int HeatPumpAirToWater::heatPumpMultiplier() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->heatPumpMultiplier();
  }

  std::string HeatPumpAirToWater::controlType() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->controlType();
  }

  double HeatPumpAirToWater::crankcaseHeaterCapacity() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->crankcaseHeaterCapacity();
  }

  boost::optional<Curve> HeatPumpAirToWater::crankcaseHeaterCapacityFunctionofTemperatureCurve() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->crankcaseHeaterCapacityFunctionofTemperatureCurve();
  }

  double HeatPumpAirToWater::maximumAmbientTemperatureforCrankcaseHeaterOperation() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->maximumAmbientTemperatureforCrankcaseHeaterOperation();
  }

  boost::optional<HeatPumpAirToWaterHeating> HeatPumpAirToWater::heatingOperationMode() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->heatingOperationMode();
  }

  boost::optional<HeatPumpAirToWaterCooling> HeatPumpAirToWater::coolingOperationMode() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->coolingOperationMode();
  }

  bool HeatPumpAirToWater::setOperatingModeControlMethod(const std::string& operatingModeControlMethod) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setOperatingModeControlMethod(operatingModeControlMethod);
  }

  bool HeatPumpAirToWater::setOperatingModeControlOptionforMultipleUnit(const std::string& operatingModeControlOptionforMultipleUnit) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setOperatingModeControlOptionforMultipleUnit(operatingModeControlOptionforMultipleUnit);
  }

  bool HeatPumpAirToWater::setOperatingModeControlSchedule(Schedule& operatingModeControlSchedule) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setOperatingModeControlSchedule(operatingModeControlSchedule);
  }

  void HeatPumpAirToWater::resetOperatingModeControlSchedule() {
    getImpl<detail::HeatPumpAirToWater_Impl>()->resetOperatingModeControlSchedule();
  }

  bool HeatPumpAirToWater::setMinimumPartLoadRatio(double minimumPartLoadRatio) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setMinimumPartLoadRatio(minimumPartLoadRatio);
  }

  bool HeatPumpAirToWater::setAirInletNodeName(const std::string& airInletNodeName) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setAirInletNodeName(airInletNodeName);
  }

  void HeatPumpAirToWater::resetAirInletNodeName() {
    getImpl<detail::HeatPumpAirToWater_Impl>()->resetAirInletNodeName();
  }

  bool HeatPumpAirToWater::setAirOutletNodeName(const std::string& airOutletNodeName) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setAirOutletNodeName(airOutletNodeName);
  }

  void HeatPumpAirToWater::resetAirOutletNodeName() {
    getImpl<detail::HeatPumpAirToWater_Impl>()->resetAirOutletNodeName();
  }

  bool HeatPumpAirToWater::setMaximumOutdoorDryBulbTemperatureForDefrostOperation(double maximumOutdoorDryBulbTemperatureForDefrostOperation) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setMaximumOutdoorDryBulbTemperatureForDefrostOperation(
      maximumOutdoorDryBulbTemperatureForDefrostOperation);
  }

  bool HeatPumpAirToWater::setHeatPumpDefrostControl(const std::string& heatPumpDefrostControl) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setHeatPumpDefrostControl(heatPumpDefrostControl);
  }

  bool HeatPumpAirToWater::setHeatPumpDefrostTimePeriodFraction(double heatPumpDefrostTimePeriodFraction) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setHeatPumpDefrostTimePeriodFraction(heatPumpDefrostTimePeriodFraction);
  }

  bool HeatPumpAirToWater::setResistiveDefrostHeaterCapacity(double resistiveDefrostHeaterCapacity) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setResistiveDefrostHeaterCapacity(resistiveDefrostHeaterCapacity);
  }

  void HeatPumpAirToWater::autosizeResistiveDefrostHeaterCapacity() {
    getImpl<detail::HeatPumpAirToWater_Impl>()->autosizeResistiveDefrostHeaterCapacity();
  }

  bool HeatPumpAirToWater::setDefrostEnergyInputRatioFunctionofTemperatureCurve(const Curve& defrostEnergyInputRatioFunctionofTemperatureCurve) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setDefrostEnergyInputRatioFunctionofTemperatureCurve(
      defrostEnergyInputRatioFunctionofTemperatureCurve);
  }

  void HeatPumpAirToWater::resetDefrostEnergyInputRatioFunctionofTemperatureCurve() {
    getImpl<detail::HeatPumpAirToWater_Impl>()->resetDefrostEnergyInputRatioFunctionofTemperatureCurve();
  }

  bool HeatPumpAirToWater::setHeatPumpMultiplier(int heatPumpMultiplier) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setHeatPumpMultiplier(heatPumpMultiplier);
  }

  bool HeatPumpAirToWater::setControlType(const std::string& controlType) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setControlType(controlType);
  }

  bool HeatPumpAirToWater::setCrankcaseHeaterCapacity(double crankcaseHeaterCapacity) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setCrankcaseHeaterCapacity(crankcaseHeaterCapacity);
  }

  bool HeatPumpAirToWater::setCrankcaseHeaterCapacityFunctionofTemperatureCurve(const Curve& crankcaseHeaterCapacityFunctionofTemperatureCurve) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setCrankcaseHeaterCapacityFunctionofTemperatureCurve(
      crankcaseHeaterCapacityFunctionofTemperatureCurve);
  }

  void HeatPumpAirToWater::resetCrankcaseHeaterCapacityFunctionofTemperatureCurve() {
    getImpl<detail::HeatPumpAirToWater_Impl>()->resetCrankcaseHeaterCapacityFunctionofTemperatureCurve();
  }

  bool HeatPumpAirToWater::setMaximumAmbientTemperatureforCrankcaseHeaterOperation(double maximumAmbientTemperatureforCrankcaseHeaterOperation) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setMaximumAmbientTemperatureforCrankcaseHeaterOperation(
      maximumAmbientTemperatureforCrankcaseHeaterOperation);
  }

  bool HeatPumpAirToWater::setHeatingOperationMode(const HeatPumpAirToWaterHeating& heatingOperationMode) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setHeatingOperationMode(heatingOperationMode);
  }

  void HeatPumpAirToWater::resetHeatingOperationMode() {
    getImpl<detail::HeatPumpAirToWater_Impl>()->resetHeatingOperationMode();
  }

  bool HeatPumpAirToWater::setCoolingOperationMode(const HeatPumpAirToWaterCooling& coolingOperationMode) {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->setCoolingOperationMode(coolingOperationMode);
  }

  void HeatPumpAirToWater::resetCoolingOperationMode() {
    getImpl<detail::HeatPumpAirToWater_Impl>()->resetCoolingOperationMode();
  }

  boost::optional<PlantLoop> HeatPumpAirToWater::coolingLoop() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->coolingLoop();
  }

  boost::optional<PlantLoop> HeatPumpAirToWater::heatingLoop() const {
    return getImpl<detail::HeatPumpAirToWater_Impl>()->heatingLoop();
  }

  /// @cond
  HeatPumpAirToWater::HeatPumpAirToWater(std::shared_ptr<detail::HeatPumpAirToWater_Impl> impl) : StraightComponent(std::move(impl)) {}
  /// @endcond

}  // namespace model
}  // namespace openstudio
