/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#ifndef MODEL_HEATPUMPAIRTOWATERHEATINGSPEEDDATA_HPP
#define MODEL_HEATPUMPAIRTOWATERHEATINGSPEEDDATA_HPP

#include "ModelAPI.hpp"
#include "ParentObject.hpp"

namespace openstudio {

namespace model {

  class Curve;

  namespace detail {

    class HeatPumpAirToWaterHeatingSpeedData_Impl;

  }  // namespace detail

  /** HeatPumpAirToWaterHeatingSpeedData is a ParentObject that wraps the OpenStudio IDD object 'OS:HeatPump:AirToWater:Heating:SpeedData'. */
  class MODEL_API HeatPumpAirToWaterHeatingSpeedData : public ParentObject
  {
   public:
    /** @name Constructors and Destructors */
    //@{

    explicit HeatPumpAirToWaterHeatingSpeedData(const Model& model);

    explicit HeatPumpAirToWaterHeatingSpeedData(const Model& model, const Curve& normalizedHeatingCapacityFunctionofTemperatureCurve,
                                                const Curve& heatingEnergyInputRatioFunctionofTemperatureCurve,
                                                const Curve& heatingEnergyInputRatioFunctionofPLRCurve);

    virtual ~HeatPumpAirToWaterHeatingSpeedData() = default;
    // Default the copy and move operators because the virtual dtor is explicit
    HeatPumpAirToWaterHeatingSpeedData(const HeatPumpAirToWaterHeatingSpeedData& other) = default;
    HeatPumpAirToWaterHeatingSpeedData(HeatPumpAirToWaterHeatingSpeedData&& other) = default;
    HeatPumpAirToWaterHeatingSpeedData& operator=(const HeatPumpAirToWaterHeatingSpeedData&) = default;
    HeatPumpAirToWaterHeatingSpeedData& operator=(HeatPumpAirToWaterHeatingSpeedData&&) = default;

    //@}

    static IddObjectType iddObjectType();

    /** @name Getters */
    //@{

    boost::optional<double> ratedHeatingCapacity() const;

    bool isRatedHeatingCapacityAutosized() const;

    double ratedCOPforHeating() const;

    Curve normalizedHeatingCapacityFunctionofTemperatureCurve() const;

    Curve heatingEnergyInputRatioFunctionofTemperatureCurve() const;

    Curve heatingEnergyInputRatioFunctionofPLRCurve() const;

    //@}
    /** @name Setters */
    //@{

    bool setRatedHeatingCapacity(double ratedHeatingCapacity);

    void autosizeRatedHeatingCapacity();

    bool setRatedCOPforHeating(double ratedCOPforHeating);

    bool setNormalizedHeatingCapacityFunctionofTemperatureCurve(const Curve& bivariateFunctions);

    bool setHeatingEnergyInputRatioFunctionofTemperatureCurve(const Curve& bivariateFunctions);

    bool setHeatingEnergyInputRatioFunctionofPLRCurve(const Curve& univariateFunctions);

    //@}
    /** @name Other */
    //@{

    // returns a vector of HeatPumpAirToWater that use this as their Speed Data
    // TODO: std::vector<HeatPumpAirToWater> heatPumpsAirToWater() const;

    boost::optional<double> autosizedRatedHeatingCapacity();

    void autosize();

    //@}
   protected:
    /// @cond
    using ImplType = detail::HeatPumpAirToWaterHeatingSpeedData_Impl;

    explicit HeatPumpAirToWaterHeatingSpeedData(std::shared_ptr<detail::HeatPumpAirToWaterHeatingSpeedData_Impl> impl);

    friend class detail::HeatPumpAirToWaterHeatingSpeedData_Impl;
    friend class Model;
    friend class IdfObject;
    friend class openstudio::detail::IdfObject_Impl;
    /// @endcond
   private:
    REGISTER_LOGGER("openstudio.model.HeatPumpAirToWaterHeatingSpeedData");
  };

  /** \relates HeatPumpAirToWaterHeatingSpeedData*/
  using OptionalHeatPumpAirToWaterHeatingSpeedData = boost::optional<HeatPumpAirToWaterHeatingSpeedData>;

  /** \relates HeatPumpAirToWaterHeatingSpeedData*/
  using HeatPumpAirToWaterHeatingSpeedDataVector = std::vector<HeatPumpAirToWaterHeatingSpeedData>;

}  // namespace model
}  // namespace openstudio

#endif  // MODEL_HEATPUMPAIRTOWATERHEATINGSPEEDDATA_HPP
