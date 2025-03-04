/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2018 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    MSVehicleType.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Thimor Bohn
/// @author  Michael Behrisch
/// @date    Tue, 06 Mar 2001
/// @version $Id$
///
// The car-following model and parameter
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <cassert>
#include <utils/iodevices/BinaryInputDevice.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/RandHelper.h>
#include <utils/common/TplConvert.h>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include <microsim/cfmodels/MSCFModel_Rail.h>
#include "MSNet.h"
#include "cfmodels/MSCFModel_IDM.h"
#include "cfmodels/MSCFModel_Kerner.h"
#include "cfmodels/MSCFModel_Krauss.h"
#include "cfmodels/MSCFModel_KraussOrig1.h"
#include "cfmodels/MSCFModel_KraussPS.h"
#include "cfmodels/MSCFModel_KraussX.h"
#include "cfmodels/MSCFModel_SmartSK.h"
#include "cfmodels/MSCFModel_Daniel1.h"
#include "cfmodels/MSCFModel_PWag2009.h"
#include "cfmodels/MSCFModel_Wiedemann.h"
#include "cfmodels/MSCFModel_ACC.h"
#include "MSVehicleControl.h"
#include "MSVehicleType.h"


// ===========================================================================
// static members
// ===========================================================================
int MSVehicleType::myNextIndex = 0;


// ===========================================================================
// method definitions
// ===========================================================================
MSVehicleType::MSVehicleType(const SUMOVTypeParameter& parameter)
    : myParameter(parameter), myWarnedActionStepLengthTauOnce(false), myIndex(myNextIndex++), myCarFollowModel(0), myOriginalType(0) {
    assert(getLength() > 0);
    assert(getMaxSpeed() > 0);

    // Check if actionStepLength was set by user, if not init to global default
    if (!myParameter.wasSet(VTYPEPARS_ACTIONSTEPLENGTH_SET)) {
        myParameter.actionStepLength = MSGlobals::gActionStepLength;
    }
    myCachedActionStepLengthSecs = STEPS2TIME(myParameter.actionStepLength);
}


MSVehicleType::~MSVehicleType() {
    delete myCarFollowModel;
}


double
MSVehicleType::computeChosenSpeedDeviation(std::mt19937* rng, const double minDev) const {
    return MAX2(minDev, myParameter.speedFactor.sample(rng));
}


// ------------ Setter methods
void
MSVehicleType::setLength(const double& length) {
    if (myOriginalType != 0 && length < 0) {
        myParameter.length = myOriginalType->getLength();
    } else {
        myParameter.length = length;
    }
    myParameter.parametersSet |= VTYPEPARS_LENGTH_SET;
}


void
MSVehicleType::setHeight(const double& height) {
    if (myOriginalType != 0 && height < 0) {
        myParameter.height = myOriginalType->getHeight();
    } else {
        myParameter.height = height;
    }
    myParameter.parametersSet |= VTYPEPARS_HEIGHT_SET;
}


void
MSVehicleType::setMinGap(const double& minGap) {
    if (myOriginalType != 0 && minGap < 0) {
        myParameter.minGap = myOriginalType->getMinGap();
    } else {
        myParameter.minGap = minGap;
    }
    myParameter.parametersSet |= VTYPEPARS_MINGAP_SET;
}


void
MSVehicleType::setMinGapLat(const double& minGapLat) {
    if (myOriginalType != 0 && minGapLat < 0) {
        myParameter.minGapLat = myOriginalType->getMinGapLat();
    } else {
        myParameter.minGapLat = minGapLat;
    }
    myParameter.parametersSet |= VTYPEPARS_MINGAP_LAT_SET;
}


void
MSVehicleType::setMaxSpeed(const double& maxSpeed) {
    if (myOriginalType != 0 && maxSpeed < 0) {
        myParameter.maxSpeed = myOriginalType->getMaxSpeed();
    } else {
        myParameter.maxSpeed = maxSpeed;
    }
    myParameter.parametersSet |= VTYPEPARS_MAXSPEED_SET;
}


void
MSVehicleType::setMaxSpeedLat(const double& maxSpeedLat) {
    if (myOriginalType != 0 && maxSpeedLat < 0) {
        myParameter.maxSpeedLat = myOriginalType->getMaxSpeedLat();
    } else {
        myParameter.maxSpeedLat = maxSpeedLat;
    }
    myParameter.parametersSet |= VTYPEPARS_MAXSPEED_LAT_SET;
}


void
MSVehicleType::setVClass(SUMOVehicleClass vclass) {
    myParameter.vehicleClass = vclass;
    myParameter.parametersSet |= VTYPEPARS_VEHICLECLASS_SET;
}

void
MSVehicleType::setPreferredLateralAlignment(LateralAlignment latAlignment) {
    myParameter.latAlignment = latAlignment;
    myParameter.parametersSet |= VTYPEPARS_LATALIGNMENT_SET;
}


void
MSVehicleType::setDefaultProbability(const double& prob) {
    if (myOriginalType != 0 && prob < 0) {
        myParameter.defaultProbability = myOriginalType->getDefaultProbability();
    } else {
        myParameter.defaultProbability = prob;
    }
    myParameter.parametersSet |= VTYPEPARS_PROBABILITY_SET;
}


void
MSVehicleType::setSpeedFactor(const double& factor) {
    if (myOriginalType != 0 && factor < 0) {
        myParameter.speedFactor.getParameter()[0] = myOriginalType->myParameter.speedFactor.getParameter()[0];
    } else {
        myParameter.speedFactor.getParameter()[0] = factor;
    }
    myParameter.parametersSet |= VTYPEPARS_SPEEDFACTOR_SET;
}


void
MSVehicleType::setSpeedDeviation(const double& dev) {
    if (myOriginalType != 0 && dev < 0) {
        myParameter.speedFactor.getParameter()[1] = myOriginalType->myParameter.speedFactor.getParameter()[1];
    } else {
        myParameter.speedFactor.getParameter()[1] = dev;
    }
    myParameter.parametersSet |= VTYPEPARS_SPEEDFACTOR_SET;
}


void
MSVehicleType::setActionStepLength(const SUMOTime actionStepLength, bool resetActionOffset) {
    assert(actionStepLength >= 0.);
    myParameter.parametersSet |= VTYPEPARS_ACTIONSTEPLENGTH_SET;

    if (myParameter.actionStepLength == actionStepLength) {
        return;
    }

    SUMOTime previousActionStepLength = myParameter.actionStepLength;
    myParameter.actionStepLength = actionStepLength;
    myCachedActionStepLengthSecs = STEPS2TIME(myParameter.actionStepLength);
    check();

    if (isVehicleSpecific()) {
        // don't perform vehicle lookup for singular vtype
        return;
    }

    // For non-singular vType reset all vehicle's actionOffsets
    // Iterate through vehicles
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    for (auto vehIt = vc.loadedVehBegin(); vehIt != vc.loadedVehEnd(); ++vehIt) {
        MSVehicle* veh = static_cast<MSVehicle*>(vehIt->second);
        if (&veh->getVehicleType() == this) {
            // Found vehicle of this type. Perform requested actionOffsetReset
            if (resetActionOffset) {
                veh->resetActionOffset();
            } else {
                veh->updateActionOffset(previousActionStepLength, actionStepLength);
            }
        }
    }
}


void
MSVehicleType::setEmissionClass(SUMOEmissionClass eclass) {
    myParameter.emissionClass = eclass;
    myParameter.parametersSet |= VTYPEPARS_EMISSIONCLASS_SET;
}


void
MSVehicleType::setColor(const RGBColor& color) {
    myParameter.color = color;
    myParameter.parametersSet |= VTYPEPARS_COLOR_SET;
}


void
MSVehicleType::setWidth(const double& width) {
    if (myOriginalType != 0 && width < 0) {
        myParameter.width = myOriginalType->getWidth();
    } else {
        myParameter.width = width;
    }
    myParameter.parametersSet |= VTYPEPARS_WIDTH_SET;
}

void
MSVehicleType::setImpatience(const double impatience) {
    if (myOriginalType != 0 && impatience < 0) {
        myParameter.impatience = myOriginalType->getImpatience();
    } else {
        myParameter.impatience = impatience;
    }
    myParameter.parametersSet |= VTYPEPARS_IMPATIENCE_SET;
}


void
MSVehicleType::setShape(SUMOVehicleShape shape) {
    myParameter.shape = shape;
    myParameter.parametersSet |= VTYPEPARS_SHAPE_SET;
}



// ------------ Static methods for building vehicle types
MSVehicleType*
MSVehicleType::build(SUMOVTypeParameter& from) {
    MSVehicleType* vtype = new MSVehicleType(from);
    const double decel = from.getCFParam(SUMO_ATTR_DECEL, SUMOVTypeParameter::getDefaultDecel(from.vehicleClass));
    const double emergencyDecel = from.getCFParam(SUMO_ATTR_EMERGENCYDECEL, SUMOVTypeParameter::getDefaultEmergencyDecel(from.vehicleClass, decel));
    // by default decel and apparentDecel are identical
    const double apparentDecel = from.getCFParam(SUMO_ATTR_APPARENTDECEL, decel);

    if (emergencyDecel < decel) {
        WRITE_WARNING("Value of 'emergencyDecel' (" + toString(emergencyDecel) + ") should be higher than 'decel' (" + toString(decel) + ") for vType '" + from.id + "'.");
    }
    if (emergencyDecel < apparentDecel) {
        WRITE_WARNING("Value of 'emergencyDecel' (" + toString(emergencyDecel) + ") is lower than 'apparentDecel' (" + toString(apparentDecel) + ") for vType '" + from.id + "' may cause collisions.");
    }

    switch (from.cfModel) {
        case SUMO_TAG_CF_IDM:
            vtype->myCarFollowModel = new MSCFModel_IDM(vtype, false);
            break;
        case SUMO_TAG_CF_IDMM:
            vtype->myCarFollowModel = new MSCFModel_IDM(vtype, true);
            break;
        case SUMO_TAG_CF_BKERNER:
            vtype->myCarFollowModel = new MSCFModel_Kerner(vtype);
            break;
        case SUMO_TAG_CF_KRAUSS_ORIG1:
            vtype->myCarFollowModel = new MSCFModel_KraussOrig1(vtype);
            break;
        case SUMO_TAG_CF_KRAUSS_PLUS_SLOPE:
            vtype->myCarFollowModel = new MSCFModel_KraussPS(vtype);
            break;
        case SUMO_TAG_CF_KRAUSSX:
            vtype->myCarFollowModel = new MSCFModel_KraussX(vtype);
            break;
        case SUMO_TAG_CF_SMART_SK:
            vtype->myCarFollowModel = new MSCFModel_SmartSK(vtype);
            break;
        case SUMO_TAG_CF_DANIEL1:
            vtype->myCarFollowModel = new MSCFModel_Daniel1(vtype);
            break;
        case SUMO_TAG_CF_PWAGNER2009:
            vtype->myCarFollowModel = new MSCFModel_PWag2009(vtype);
            break;
        case SUMO_TAG_CF_WIEDEMANN:
            vtype->myCarFollowModel = new MSCFModel_Wiedemann(vtype);
            break;
        case SUMO_TAG_CF_RAIL:
            vtype->myCarFollowModel = new MSCFModel_Rail(vtype);
            break;
        case SUMO_TAG_CF_ACC:
            vtype->myCarFollowModel = new MSCFModel_ACC(vtype);
            break;
        case SUMO_TAG_CF_KRAUSS:
        default:
            vtype->myCarFollowModel = new MSCFModel_Krauss(vtype);
            break;
    }
    vtype->check();
    return vtype;
}


MSVehicleType*
MSVehicleType::buildSingularType(const std::string& id) const {
    return duplicateType(id, false);
}


MSVehicleType*
MSVehicleType::duplicateType(const std::string& id, bool persistent) const {
    MSVehicleType* vtype = new MSVehicleType(myParameter);
    vtype->myParameter.id = id;
    vtype->myCarFollowModel = myCarFollowModel->duplicate(vtype);
    if (!persistent) {
        vtype->myOriginalType = this;
    }
    if (!MSNet::getInstance()->getVehicleControl().addVType(vtype)) {
        std::string singular = persistent ? "" : "singular ";
        throw ProcessError("could not add " + singular + "type " + vtype->getID());
    }
    return vtype;
}

void
MSVehicleType::check() {
    if (!myWarnedActionStepLengthTauOnce
            && myParameter.wasSet(VTYPEPARS_ACTIONSTEPLENGTH_SET)
            && STEPS2TIME(myParameter.actionStepLength) > getCarFollowModel().getHeadwayTime()) {
        myWarnedActionStepLengthTauOnce = true;
        std::stringstream s;
        s << "Given action step length " << STEPS2TIME(myParameter.actionStepLength) << " for vehicle type '" << getID()
          << "' is larger than its parameter tau (=" << getCarFollowModel().getHeadwayTime() << ")!"
          << " This may lead to collisions. (This warning is only issued once per vehicle type).";
        WRITE_WARNING(s.str());
    }
}

void
MSVehicleType::setAccel(double accel) {
    if (myOriginalType != 0 && accel < 0) {
        accel = myOriginalType->getCarFollowModel().getMaxAccel();
    }
    myCarFollowModel->setMaxAccel(accel);
    myParameter.cfParameter[SUMO_ATTR_ACCEL] = toString(accel);
}

void
MSVehicleType::setDecel(double decel) {
    if (myOriginalType != 0 && decel < 0) {
        decel = myOriginalType->getCarFollowModel().getMaxDecel();
    }
    myCarFollowModel->setMaxDecel(decel);
    myParameter.cfParameter[SUMO_ATTR_DECEL] = toString(decel);
}

void
MSVehicleType::setEmergencyDecel(double emergencyDecel) {
    if (myOriginalType != 0 && emergencyDecel < 0) {
        emergencyDecel = myOriginalType->getCarFollowModel().getEmergencyDecel();
    }
    myCarFollowModel->setEmergencyDecel(emergencyDecel);
    myParameter.cfParameter[SUMO_ATTR_EMERGENCYDECEL] = toString(emergencyDecel);
}

void
MSVehicleType::setApparentDecel(double apparentDecel) {
    if (myOriginalType != 0 && apparentDecel < 0) {
        apparentDecel = myOriginalType->getCarFollowModel().getApparentDecel();
    }
    myCarFollowModel->setApparentDecel(apparentDecel);
    myParameter.cfParameter[SUMO_ATTR_APPARENTDECEL] = toString(apparentDecel);
}

void
MSVehicleType::setImperfection(double imperfection) {
    if (myOriginalType != 0 && imperfection < 0) {
        imperfection = myOriginalType->getCarFollowModel().getImperfection();
    }
    myCarFollowModel->setImperfection(imperfection);
    myParameter.cfParameter[SUMO_ATTR_SIGMA] = toString(imperfection);
}

void
MSVehicleType::setTau(double tau) {
    if (myOriginalType != 0 && tau < 0) {
        tau = myOriginalType->getCarFollowModel().getHeadwayTime();
    }
    myCarFollowModel->setHeadwayTime(tau);
    myParameter.cfParameter[SUMO_ATTR_TAU] = toString(tau);
}


/****************************************************************************/

