/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkFiniteDifferenceFunction.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef __itkFiniteDifferenceFunction_h_
#define __itkFiniteDifferenceFunction_h_

#include "itkLightObject.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkConstSmartNeighborhoodIterator.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkVector.h"

namespace itk {

/**
 * \class FiniteDifferenceFunction
 *
 * This class is a component object of the finite difference solver hierarchy
 * (see itkFiniteDifferenceImageFilter).  It defines a generic interface for a
 * function object that computes a single scalar value from a neighborhood of
 * values.  Examples of the application of this class are the various flavors
 * of AnisotropicDiffusionFunctions and LevelSetFunction objects.
 *
 * These functions calculate the incremental change at a pixel in the solution
 * image from one iteration of the p.d.e. solver to the next.
 * 
 * \par
 * Subclasses of FiniteDifferenceImageFilter (solvers) call the
 * ComputeUpdate() method of this class to compute \f$ \Delta u^n_{\mathbf{i}}
 * \f$ at each \f$ i \f$.  in \f$ u \f$.  Because the size of the time step for
 * each iteration of the p.d.e. solution depends on the particular calculations
 * done, this function object is also responsible for computing that time step
 * (see ComputeGlobalTimeStep).
 *
 * \par How to use this class
 * FiniteDifferenceFunction must be subclassed to add functionality for
 * ComputeUpdate, ComputeGlobalTimeStep, and Get/ReleaseGlobalDataPointer.
 *
 * \par A note on thread safety.
 * The ComputeUpdate() methods of this filter are declared as const to enforce
 * thread-safety during execution of FiniteDifferenceImageFilter solver
 * algorithms.  The InitializeIteration() method is intended to provide a safe
 * way to modify the state of the object between threaded calculations of
 * solvers.
 *
 * \todo Possibly subclass this object from Function.  Stumbling blocks here
 * are the specialized api of FiniteDifferenceFunction.
 *
 * \ingroup Functions */
template<class TImageType>
class FiniteDifferenceFunction : public LightObject
{
public:
  /** Standard class typedefs. */
  typedef FiniteDifferenceFunction Self;
  typedef LightObject Superclass;
  
  /** Run-time type information (and related methods) */
  itkTypeMacro( FiniteDifferenceFunction, LightObject );

  /** Extract some parameters from the image type */
  typedef TImageType ImageType;
  typedef typename ImageType::PixelType       PixelType;
  
  /** Save image dimension. */
  enum { ImageDimension = ImageType::ImageDimension };

  /** Define the TimeStepType to always be double. */
  typedef double  TimeStepType;

  /** The default boundary condition for finite difference
   * functions that is used unless overridden in the Evaluate() method. */
  typedef ZeroFluxNeumannBoundaryCondition<ImageType>
    DefaultBoundaryConditionType;

  /** Neighborhood radius type */
  typedef typename ConstNeighborhoodIterator<TImageType>::RadiusType RadiusType;

  /** The type of data structure that is passed to this function object
   * to evaluate at a pixel that does not lie on a data set boundary. */
  typedef ConstNeighborhoodIterator<TImageType> NeighborhoodType;

  /** The type of data structure that is passed to this function object
   * to evaluate at a pixel that lies on a data set boundary. */
  typedef ConstSmartNeighborhoodIterator<TImageType,
    DefaultBoundaryConditionType> BoundaryNeighborhoodType;

  /** A floating point offset from an image grid location. Used for
   * interpolation among grid values in a neighborhood. */
  typedef Vector<float, ImageDimension> FloatOffsetType;
  
  /** This method allows the function to set its state before each iteration
   *  of the finite difference solver (image filter) that uses it.  This is
   *  a thread-safe time to manipulate the object's state.
   *
   * An example of how this can be used: the Anisotropic diffusion class of
   * FiniteDifferenceFunctions use this method to pre-calculate an average
   * gradient magnitude across the entire image region.  This value is set in
   * the function object and used by the ComputeUpdate methods that are called
   * at each pixel as a constant. */
  virtual void InitializeIteration() {};

  /** This method is called by a finite difference solver image filter at
   * each pixel that does not lie on a data set boundary.  The width of the
   * data set boundary is defined by the width of the neighborhood being
   * evaluated.
   *
   * The neighborhood argument is the neighborhood data.
   * The globalData argument is a pointer to a data structure that holds
   * values that need to be persistent across calls to this function, but
   * cannot be stored in the function object itself for thread-safety reasons.
   * Examples are values needed to compute the time-step for an iteration
   * of the solver.
   * \sa InitializeIteration
   * \sa ComputeGlobalTimeStep */
  virtual PixelType  ComputeUpdate(const NeighborhoodType &neighborhood,
                                   void *globalData,
                                   const FloatOffsetType &offset = m_ZeroOffset)
    const = 0;

  /** This method is called by a finite difference solver image filter at
   * each pixel that lies on the data set boundary.
   *
   * See its overloaded counterpart for more information. */
  virtual PixelType  ComputeUpdate(const BoundaryNeighborhoodType
                                   &neighborhood, void *globalData,
                              const FloatOffsetType &offset = m_ZeroOffset)
                                  const = 0;

  /** Sets the radius of the neighborhood this FiniteDifferenceFunction
   * needs to perform its calculations. */
  void SetRadius(const RadiusType &r)
    { m_Radius = r; }

  /** Returns the radius of the neighborhood this FiniteDifferenceFunction
   * needs to perform its calculations. */
  const RadiusType &GetRadius() const
    { return m_Radius; }

  /** Computes the time step for an update given a global data structure.
   * The data used in the computation may take different forms depending on
   * the nature of the equations.  This global data cannot be kept in the
   * instance of the equation object itself since the equation object must
   * remain stateless for thread safety.  The global data is therefore managed
   * for each thread by the finite difference solver filters. */
  virtual TimeStepType ComputeGlobalTimeStep(void *GlobalData) const =0;

  /** Returns a pointer to a global data structure that is passed to this
   * object from the solver at each calculation.  The idea is that the
   * solver holds the state of any global values needed to calculate the
   * time step, while the equation object performs the actual calculations.
   *
   * The global data should also be initialized in this method.
   * */
  virtual void *GetGlobalDataPointer() const =0;

  /** When the finite difference solver filter has finished using a global
   * data pointer, it passes it to this method, which frees the memory.
   *
   * The solver cannot free the memory because it does not know the type
   * to which the pointer points. */
  virtual void ReleaseGlobalDataPointer(void *GlobalData) const =0;
  
protected:
  FiniteDifferenceFunction() {}
  ~FiniteDifferenceFunction() {}
  void PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
    os << indent << "Radius: " << m_Radius << std::endl;
  }

  RadiusType m_Radius;

  /** An internal constant to represent an n-dimensional floating-point
   * grid offset of zero. */
  static FloatOffsetType m_ZeroOffset;

private:
  FiniteDifferenceFunction(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** Used internally for the sole purpose of initialization of the static
   * variable m_ZeroOffset  */
  static FloatOffsetType InitializeZeroOffset();

};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFiniteDifferenceFunction.txx"
#endif

#endif
