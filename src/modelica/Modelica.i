#ifndef MODELICA_I
#define MODELICA_I

#ifdef SWIGPYTHON
%module(directors="1") openstudiomodelica
#endif

%include <utilities/UtilitiesAPI.hpp>
#define MODELICA_API

%include <utilities/core/CommonInclude.i>
%import <utilities/core/CommonImport.i>
%import <utilities/Utilities.i>

// Include path support for openstudio::path conversions
%include <utilities/core/Path.i>

%{
  #include <utilities/core/Path.hpp>
  #include <utilities/core/PathHelpers.hpp>
  #include <modelica/ModelicaFile.hpp>
  
  using namespace openstudio;
  using namespace openstudio::modelica;
%}

// Create vector templates for Ruby arrays
%ignore std::vector<openstudio::modelica::ClassDefinition>::vector(size_type);
%ignore std::vector<openstudio::modelica::ClassDefinition>::resize(size_type);
%ignore std::vector<openstudio::modelica::ConnectClause>::vector(size_type);
%ignore std::vector<openstudio::modelica::ConnectClause>::resize(size_type);

%template(ClassDefinitionVector) std::vector<openstudio::modelica::ClassDefinition>;
%template(OptionalClassDefinition) boost::optional<openstudio::modelica::ClassDefinition>;
%template(ConnectClauseVector) std::vector<openstudio::modelica::ConnectClause>;

%include <modelica/ModelicaFile.hpp>

#endif // MODELICA_I
