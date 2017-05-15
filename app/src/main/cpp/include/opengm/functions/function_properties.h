#pragma once
#ifndef OPENGM_FUNCTION_PROPERTIES_HXX
#define OPENGM_FUNCTION_PROPERTIES_HXX

#include <cmath>

#include "opengm/opengm.h"
#include "opengm/utilities/shape_accessor.h"
#include "opengm/utilities/accessor_iterator.h"
#include "opengm/utilities/accumulation.h"
#include "opengm/utilities/indexing.h"
#include "opengm/utilities/functors.h"
#include "opengm/operations/adder.h"
#include "opengm/operations/and.h"
#include "opengm/operations/or.h"
#include "opengm/operations/minimizer.h"
#include "opengm/operations/maximizer.h"
#include "opengm/operations/adder.h"
#include "opengm/operations/integrator.h"
#include "opengm/operations/multiplier.h"

#define OPENGM_FUNCTION_TYPEDEF_MACRO typedef typename FunctionType::ValueType ValueType;\
typedef typename FunctionType::IndexType IndexType;\
typedef typename FunctionType::LabelType LabelType;\
typedef typename FunctionType::FunctionShapeIteratorType FunctionShapeIteratorType

namespace opengm {

struct BinaryProperties{
   enum Values{
       IsPotts=0,
       IsSubmodular1=1,
       IsPositive=2
   };
};

struct ValueProperties{
   enum Values{
      Sum=0,
      Product=1,
      Minimum=2,
      Maximum=3,
      FeasibleValue
   };
};

template<int PROPERTY_ID,class FUNCTION>
class BinaryFunctionProperties;

template<int PROPERTY_ID,class FUNCTION>
class ValueFunctionProperties;

namespace detail_properties{
   template<class FUNCTION,class ACCUMULATOR>
   class AllValuesInAnyOrderFunctionProperties;
   template<class FUNCTION,class ACCUMULATOR>
   class AtLeastAllUniqueValuesFunctionProperties;
}

// Fallback implementation(s) of binary properties
template<class FUNCTION>
class BinaryFunctionProperties<BinaryProperties::IsPotts, FUNCTION> {
   typedef FUNCTION FunctionType;
   OPENGM_FUNCTION_TYPEDEF_MACRO;
public:
   static bool op(const FunctionType & f) {
      ShapeWalker<FunctionShapeIteratorType> shapeWalker(f.functionShapeBegin(), f.dimension());
      ValueType vEqual = f(shapeWalker.coordinateTuple().begin());
      ++shapeWalker;
      ValueType vNotEqual = f(shapeWalker.coordinateTuple().begin());
      ++shapeWalker;
      for (IndexType i = 2; i < f.size(); ++i, ++shapeWalker) {
         // all labels are equal
         if (isEqualValueVector(shapeWalker.coordinateTuple())) {
            if (vEqual != f(shapeWalker.coordinateTuple().begin()))
               return false;
         }               // all labels are not equal
         else {
            if (vNotEqual != f(shapeWalker.coordinateTuple().begin()))
               return false;
         }
      }
      return true;
   }
};


// Fallback implementation(s) of (real) value properties
// Some basic properties are derived from 
// "AllValuesInAnyOrderFunctionProperties" and 
// "AtLeastAllUniqueValuesFunctionProperties"
template<class FUNCTION>
class ValueFunctionProperties<ValueProperties::Product, FUNCTION> 
: public  detail_properties::AllValuesInAnyOrderFunctionProperties<FUNCTION,Multiplier>{
}; 

template<class FUNCTION>
class ValueFunctionProperties<ValueProperties::Sum, FUNCTION> 
 : public  detail_properties::AllValuesInAnyOrderFunctionProperties<FUNCTION,Adder>{
}; 

template<class FUNCTION>
class ValueFunctionProperties<ValueProperties::Minimum, FUNCTION> 
 : public detail_properties::AtLeastAllUniqueValuesFunctionProperties<FUNCTION,Minimizer>{
}; 

template<class FUNCTION>
class ValueFunctionProperties<ValueProperties::Maximum, FUNCTION> 
  : public detail_properties::AtLeastAllUniqueValuesFunctionProperties<FUNCTION,Maximizer>{
}; 


namespace detail_properties{
   template<class FUNCTION,class ACCUMULATOR>
   class AllValuesInAnyOrderFunctionProperties{
      typedef FUNCTION FunctionType;
      OPENGM_FUNCTION_TYPEDEF_MACRO;
   public:
      static ValueType op(const FunctionType & f) {
         opengm::AccumulationFunctor<ACCUMULATOR,ValueType> functor;
         f.forAllValuesInAnyOrder(functor);
         return functor.value();
      }
   };
   template<class FUNCTION,class ACCUMULATOR>
   class AtLeastAllUniqueValuesFunctionProperties{
      typedef FUNCTION FunctionType;
      OPENGM_FUNCTION_TYPEDEF_MACRO;
   public:
      static ValueType op(const FunctionType & f) {
         opengm::AccumulationFunctor<ACCUMULATOR,ValueType> functor;
         f.forAllValuesInAnyOrder(functor);
         return functor.value();
      }
   };
}

}// namespace opengm

#endif //OPENGM_FUNCTION_PROPERTIES_HXX
