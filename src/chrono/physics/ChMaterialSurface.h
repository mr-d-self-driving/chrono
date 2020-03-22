// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Alessandro Tasora, Radu Serban
// =============================================================================

#ifndef CH_MATERIAL_SURFACE_H
#define CH_MATERIAL_SURFACE_H

#include <algorithm>

#include "chrono/core/ChClassFactory.h"
#include "chrono/serialization/ChArchive.h"

namespace chrono {

/// Enumeration of contact methods.
enum class ChContactMethod {
    NSC,  ///< non-smooth, constraint-based (a.k.a. rigid-body) contact
    SMC   ///< smooth, penalty-based (a.k.a. soft-body) contact
};

/// Base class for specifying material properties for contact force generation.
class ChApi ChMaterialSurface {
  public:
    virtual ~ChMaterialSurface() {}

    /// "Virtual" copy constructor.
    virtual ChMaterialSurface* Clone() const = 0;

    virtual ChContactMethod GetContactMethod() const = 0;

    virtual void ArchiveOUT(ChArchiveOut& marchive) {
        // version number:
        marchive.VersionWrite<ChMaterialSurface>();
    }

    virtual void ArchiveIN(ChArchiveIn& marchive) {
        // version number:
        int version = marchive.VersionRead<ChMaterialSurface>();
    }
};

CH_CLASS_VERSION(ChMaterialSurface, 0)

/// Base class for composite material for a contact pair.
class ChApi ChMaterialComposite {
  public:
    virtual ~ChMaterialComposite() {}
};

/// Base class for material composition strategy.
/// Implements the default combination laws for coefficients of friction, cohesion, compliance, etc.
/// Derived classes can override one or more of these combination laws.
/// Enabling the use of a customized composition strategy is system type-dependent.
class ChApi ChMaterialCompositionStrategy {
  public:
    virtual ~ChMaterialCompositionStrategy() {}

    virtual float CombineFriction(float a1, float a2) const { return std::min<float>(a1, a2); }
    virtual float CombineCohesion(float a1, float a2) const { return std::min<float>(a1, a2); }
    virtual float CombineRestitution(float a1, float a2) const { return std::min<float>(a1, a2); }
    virtual float CombineDamping(float a1, float a2) const { return std::min<float>(a1, a2); }
    virtual float CombineCompliance(float a1, float a2) const { return a1 + a2; }

    virtual float CombineAdhesionMultiplier(float a1, float a2) const { return std::min<float>(a1, a2); }
    virtual float CombineStiffnessCoefficient(float a1, float a2) const { return (a1 + a2) / 2; }
    virtual float CombineDampingCoefficient(float a1, float a2) const { return (a1 + a2) / 2; }
};

}  // end namespace chrono

#endif
