/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#ifndef MODEL_HEATPUMPAIRTOWATERHEATINGSPEEDDATA_IMPL_HPP
#define MODEL_HEATPUMPAIRTOWATERHEATINGSPEEDDATA_IMPL_HPP

#include "ModelAPI.hpp"
#include "ParentObject_Impl.hpp"

namespace openstudio {
namespace model {

  class Curve;

  namespace detail {

    /** HeatPumpAirToWaterHeatingSpeedData_Impl is a ParentObject_Impl that is the implementation class for HeatPumpAirToWaterHeatingSpeedData.*/
    class MODEL_API HeatPumpAirToWaterHeatingSpeedData_Impl : public ParentObject_Impl
    {
     public:
      /** @name Constructors and Destructors */
      //@{

      HeatPumpAirToWaterHeatingSpeedData_Impl(const IdfObject& idfObject, Model_Impl* model, bool keepHandle);

      HeatPumpAirToWaterHeatingSpeedData_Impl(const openstudio::detail::WorkspaceObject_Impl& other, Model_Impl* model, bool keepHandle);

      HeatPumpAirToWaterHeatingSpeedData_Impl(const HeatPumpAirToWaterHeatingSpeedData_Impl& other, Model_Impl* model, bool keepHandle);

      virtual ~HeatPumpAirToWaterHeatingSpeedData_Impl() = default;

      //@}
      /** @name Virtual Methods */
      //@{

      virtual const std::vector<std::string>& outputVariableNames() const override;

      virtual IddObjectType iddObjectType() const override;

      // Overriding clone and children here because we want to try to remove the Curves (if they aren't used by something else)
      // So we list them as children. But ParentObject_Impl::clone would also clone them, so we override clone to call ModelObject_Impl::clone
      virtual ModelObject clone(Model model) const override;

      virtual std::vector<ModelObject> children() const override;

      //@}
      /** @name Getters */
      //@{

      boost::optional<double> ratedHeatingCapacity() const;

      bool isRatedHeatingCapacityAutosized() const;

      boost::optional<double> autosizedRatedHeatingCapacity();

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

      void autosize();

      void applySizingValues();

      //@}
     protected:
     private:
      REGISTER_LOGGER("openstudio.model.HeatPumpAirToWaterHeatingSpeedData");

      // Optional getters for use by methods like children() so can remove() if the constructor fails.
      // There are other ways for the public versions of these getters to fail--perhaps all required
      // objects should be returned as boost::optionals
      boost::optional<Curve> optionalNormalizedHeatingCapacityFunctionofTemperatureCurve() const;
      boost::optional<Curve> optionalHeatingEnergyInputRatioFunctionofTemperatureCurve() const;
      boost::optional<Curve> optionalHeatingEnergyInputRatioFunctionofPLRCurve() const;
    };

  }  // namespace detail

}  // namespace model
}  // namespace openstudio

#endif  // MODEL_HEATPUMPAIRTOWATERHEATINGSPEEDDATA_IMPL_HPP
