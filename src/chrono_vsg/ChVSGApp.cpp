// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2020 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Rainer Gericke
// =============================================================================
// Vulkan Scene Graph viewer, this class will hopefully draw the system to the
// screen and handle input some day
// =============================================================================

#include "chrono_vsg/core/ChApiVSG.h"

#include "ChVSGApp.h"
#include "chrono/geometry/ChBox.h"
#include "chrono/assets/ChColorAsset.h"
#include "chrono/assets/ChBoxShape.h"
#include "chrono/assets/ChSphereShape.h"
#include "chrono/assets/ChEllipsoidShape.h"
#include "chrono/assets/ChCylinderShape.h"
#include "chrono_vsg/shapes/ChVSGBox.h"
#include "chrono_vsg/shapes/ChVSGSphere.h"
#include "chrono_vsg/shapes/ChVSGCylinder.h"

using namespace chrono::vsg3d;

ChVSGApp::ChVSGApp() : m_horizonMountainHeight(0.0) {}

bool ChVSGApp::Initialize(int windowWidth, int windowHeight, const char* windowTitle, ChSystem* system) {
    if (!system) {
        return false;
    }
    m_system = system;
    m_windowTraits = ::vsg::WindowTraits::create();
    m_windowTraits->windowTitle = windowTitle;
    m_windowTraits->width = windowWidth;
    m_windowTraits->height = windowHeight;
    m_windowTraits->x = 100;
    m_windowTraits->y = 100;

    m_searchPaths = ::vsg::getEnvPaths("VSG_FILE_PATH");

    m_scenegraph = vsg::Group::create();

    // fill the scenegraph with asset definitions from the physical system
    AnalyseSystem();

    // create viewer
    m_viewer = ::vsg::Viewer::create();

    // create window
    m_window = ::vsg::Window::create(m_windowTraits);
    // if (!window) {
    if (!m_window) {
        GetLog() << "Could not create windows.\n";
        return false;
    }

    m_viewer->addWindow(m_window);

    // compute the bounds of the scene graph to help position camera
    vsg::ComputeBounds computeBounds;
    m_scenegraph->accept(computeBounds);
    vsg::dvec3 centre = (computeBounds.bounds.min + computeBounds.bounds.max) * 0.5;
    double radius = vsg::length(computeBounds.bounds.max - computeBounds.bounds.min) * 0.6;
    double nearFarRatio = 0.001;
    GetLog() << "BoundMin = {" << computeBounds.bounds.min.x << ";" << computeBounds.bounds.min.y << ";"
             << computeBounds.bounds.min.z << "}\n";
    GetLog() << "BoundMax = {" << computeBounds.bounds.max.x << ";" << computeBounds.bounds.max.y << ";"
             << computeBounds.bounds.max.z << "}\n";
    // set up the camera
    auto lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, -radius * 3.5, 0.0), centre, vsg::dvec3(0.0, 0.0, 1.0));

    auto perspective = vsg::Perspective::create(
        30.0, static_cast<double>(m_window->extent2D().width) / static_cast<double>(m_window->extent2D().height),
        nearFarRatio * radius, radius * 4.5);

    auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(m_window->extent2D()));

    // add close handler to respond to pressing the window close window button and pressing escape
    m_viewer->addEventHandler(::vsg::CloseHandler::create(m_viewer));

    // add a trackball event handler to control the camera view use the mouse
    m_viewer->addEventHandler(::vsg::Trackball::create(camera));

    auto commandGraph = vsg::createCommandGraphForView(m_window, camera, m_scenegraph);
    m_viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

    m_viewer->compile();
    return true;
}

ChVSGApp::~ChVSGApp() {
    ;
}

void ChVSGApp::Render() {
    m_viewer->handleEvents();
    m_viewer->update();
    m_viewer->recordAndSubmit();
    m_viewer->present();
}

void ChVSGApp::AnalyseSystem() {
    // analyse system, look for bodies and assets
    for (auto body : m_system->Get_bodylist()) {
        GetLog() << "ChVSGApp::ChVSGApp(): Body " << body << "\n";
        // position of the body
        const Vector pos = body->GetFrame_REF_to_abs().GetPos();
        // rotation of the body
        Quaternion rot = body->GetFrame_REF_to_abs().GetRot();
        double angle;
        Vector axis;
        rot.Q_to_AngAxis(angle, axis);
        for (int i = 0; i < body->GetAssets().size(); i++) {
            auto asset = body->GetAssets().at(i);

            vsg::vec4 color(1, 1, 1, 1);
            if (std::dynamic_pointer_cast<ChColorAsset>(asset)) {
                ChColorAsset* color_asset = (ChColorAsset*)(asset.get());
                color[0] = color_asset->GetColor().R;
                color[1] = color_asset->GetColor().G;
                color[2] = color_asset->GetColor().B;
                color[3] = color_asset->GetColor().A;
            }
            if (!std::dynamic_pointer_cast<ChVisualization>(asset)) {
                continue;
            }
            ChVisualization* visual_asset = ((ChVisualization*)(asset.get()));
            // position of the asset
            Vector center = visual_asset->Pos;
            // rotate asset pos into global frame
            center = rot.Rotate(center);
            // Get the local rotation of the asset
            Quaternion lrot = visual_asset->Rot.Get_A_quaternion();
            // add the local rotation to the rotation of the body
            lrot = rot % lrot;
            lrot.Normalize();
            lrot.Q_to_AngAxis(angle, axis);
            if (ChBoxShape* box_shape = dynamic_cast<ChBoxShape*>(asset.get())) {
                GetLog() << "Found BoxShape!\n";
                ChVector<> size = box_shape->GetBoxGeometry().GetSize();
                ChVector<> pos_final = pos + center;
                // GetLog() << "pos final = " << pos_final << "\n";

                // model = glm::translate(glm::mat4(1), glm::vec3(pos_final.x(), pos_final.y(), pos_final.z()));
                // model = glm::rotate(model, float(angle), glm::vec3(axis.x(), axis.y(), axis.z()));
                // model = glm::scale(model, glm::vec3(radius, radius, radius));
                // model_sphere.push_back(model);
                auto transform = vsg::MatrixTransform::create();

                transform->setMatrix(vsg::translate(pos_final.x(), pos_final.y(), pos_final.z()) *
                                     vsg::scale(size.x(), size.y(), size.z()) *
                                     vsg::rotate(angle, axis.x(), axis.y(), axis.z()));

                ChVSGBox theBox;
                vsg::ref_ptr<vsg::Node> node = theBox.createTexturedNode(color, transform);
                m_scenegraph->addChild(node);
                m_transformList.push_back(transform);  // we will need it later
            }
            if (ChSphereShape* sphere_shape = dynamic_cast<ChSphereShape*>(asset.get())) {
                GetLog() << "Found SphereShape!\n";
                double radius = sphere_shape->GetSphereGeometry().rad;
                ChVector<> size(radius, radius, radius);
                ChVector<> pos_final = pos + center;
                // GetLog() << "pos final = " << pos_final << "\n";

                // model = glm::translate(glm::mat4(1), glm::vec3(pos_final.x(), pos_final.y(), pos_final.z()));
                // model = glm::rotate(model, float(angle), glm::vec3(axis.x(), axis.y(), axis.z()));
                // model = glm::scale(model, glm::vec3(radius, radius, radius));
                // model_sphere.push_back(model);
                auto transform = vsg::MatrixTransform::create();

                transform->setMatrix(vsg::translate(pos_final.x(), pos_final.y(), pos_final.z()) *
                                     vsg::scale(size.x(), size.y(), size.z()) *
                                     vsg::rotate(angle, axis.x(), axis.y(), axis.z()));

                ChVSGSphere theSphere;
                vsg::ref_ptr<vsg::Node> node = theSphere.createTexturedNode(color, transform);
                m_scenegraph->addChild(node);
                m_transformList.push_back(transform);  // we will need it later
            }
            if (ChEllipsoidShape* ellipsoid_shape = dynamic_cast<ChEllipsoidShape*>(asset.get())) {
                GetLog() << "Found ElipsoidShape!\n";
                Vector radius = ellipsoid_shape->GetEllipsoidGeometry().rad;
                ChVector<> size(radius.x(), radius.y(), radius.z());
                ChVector<> pos_final = pos + center;
                // GetLog() << "pos final = " << pos_final << "\n";

                // model = glm::translate(glm::mat4(1), glm::vec3(pos_final.x(), pos_final.y(), pos_final.z()));
                // model = glm::rotate(model, float(angle), glm::vec3(axis.x(), axis.y(), axis.z()));
                // model = glm::scale(model, glm::vec3(radius, radius, radius));
                // model_sphere.push_back(model);
                auto transform = vsg::MatrixTransform::create();

                transform->setMatrix(vsg::translate(pos_final.x(), pos_final.y(), pos_final.z()) *
                                     vsg::scale(size.x(), size.y(), size.z()) *
                                     vsg::rotate(angle, axis.x(), axis.y(), axis.z()));

                ChVSGSphere theSphere;
                vsg::ref_ptr<vsg::Node> node = theSphere.createTexturedNode(color, transform);
                m_scenegraph->addChild(node);
                m_transformList.push_back(transform);  // we will need it later
            }
            if (ChCylinderShape* cylinder_shape = dynamic_cast<ChCylinderShape*>(asset.get())) {
                GetLog() << "Found CylinderShape!\n";
                double rad = cylinder_shape->GetCylinderGeometry().rad;
                ChVector<> dir = cylinder_shape->GetCylinderGeometry().p2 - cylinder_shape->GetCylinderGeometry().p1;
                double height = dir.Length();
                dir.Normalize();
                ChVector<> mx, my, mz;
                dir.DirToDxDyDz(my, mz, mx);  // y is axis, in cylinder.obj frame
                ChMatrix33<> mrot;
                mrot.Set_A_axis(mx, my, mz);
                lrot = rot % (visual_asset->Rot.Get_A_quaternion() % mrot.Get_A_quaternion());
                // position of cylinder based on two points
                ChVector<> mpos =
                    0.5 * (cylinder_shape->GetCylinderGeometry().p2 + cylinder_shape->GetCylinderGeometry().p1);

                lrot.Q_to_AngAxis(angle, axis);
                ChVector<> pos_final = pos + rot.Rotate(mpos);
                auto transform = vsg::MatrixTransform::create();

                transform->setMatrix(vsg::translate(pos_final.x(), pos_final.y(), pos_final.z()) *
                                     vsg::scale(rad, rad, height) * vsg::rotate(angle, axis.x(), axis.y(), axis.z()));

                ChVSGCylinder theCylinder;
                vsg::ref_ptr<vsg::Node> node = theCylinder.createTexturedNode(color, transform);
                m_scenegraph->addChild(node);
                m_transformList.push_back(transform);  // we will need it later
            }
        }
    }
}