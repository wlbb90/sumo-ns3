/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2018 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    GNEDetectorE1Instant.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2018
/// @version $Id$
///
//
/****************************************************************************/
#ifndef GNEDetectorE1Instant_h
#define GNEDetectorE1Instant_h


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include "GNEDetector.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEDetectorE1Instant
 * Class for detector of type E1
 */
class GNEDetectorE1Instant : public GNEDetector {

public:
    /**@brief Constructor
     * @param[in] id The storage of gl-ids to get the one for this lane representation from
     * @param[in] lane Lane of this StoppingPlace belongs
     * @param[in] viewNet pointer to GNEViewNet of this additional element belongs
     * @param[in] pos position of the detector on the lane
     * @param[in] filename The path to the output file.
     * @param[in] vehicleTypes space separated list of vehicle type ids to consider
     * @param[in] name E1 Instant detector name
     * @param[in] friendlyPos enable or disable friendly positions
     * @param[in] block movement enable or disable additional movement
     */
    GNEDetectorE1Instant(const std::string& id, GNELane* lane, GNEViewNet* viewNet, double pos, const std::string& filename, const std::string& vehicleTypes, const std::string& name, bool friendlyPos, bool blockMovement);

    /// @brief Destructor
    ~GNEDetectorE1Instant();

    /// @brief check if Position of detector is fixed
    bool isDetectorPositionFixed() const;

    /// @name Functions related with geometry of element
    /// @{
    /// @brief update pre-computed geometry information
    void updateGeometry(bool updateGrid);
    /// @}

    /// @name inherited from GUIGlObject
    /// @{
    /**@brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;
    /// @}

    /// @name inherited from GNEAttributeCarrier
    /// @{
    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    std::string getAttribute(SumoXMLAttr key) const;

    /* @brief method for setting the attribute and letting the object perform additional changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     */
    void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList);

    /* @brief method for checking if the key and their correspond attribute are valids
     * @param[in] key The attribute key
     * @param[in] value The value asociated to key key
     * @return true if the value is valid, false in other case
     */
    bool isValid(SumoXMLAttr key, const std::string& value);
    /// @}

private:
    /// @brief set attribute after validation
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief Invalidated copy constructor.
    GNEDetectorE1Instant(const GNEDetectorE1Instant&) = delete;

    /// @brief Invalidated assignment operator.
    GNEDetectorE1Instant& operator=(const GNEDetectorE1Instant&) = delete;
};

#endif
/****************************************************************************/
