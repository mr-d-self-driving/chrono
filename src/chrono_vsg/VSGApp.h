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
// Vulkan Scene Graph viewer class
// =============================================================================

#ifndef VSG_APP_H
#define VSG_APP_H

#include <vsg/all.h>

#include "chrono_vsg/core/ChApiVSG.h"
#include "chrono_vsg/resources/ChVSGSettings.h"
#include "chrono_vsg/tools/VSGScreenshotHandler.h"
#include "chrono/physics/ChSystem.h"

namespace chrono {
namespace vsg3d {

/// @addtogroup vsg_module
/// @{

class CH_VSG_API VSGApp {
  public:
    VSGApp();

    ~VSGApp();

    bool Initialize(int windowWidth, int windowHeight, const char* windowTitle, ChSystem* system);

    void Render();

    void Quit();

    vsg::ref_ptr<vsg::Viewer> GetViewer() const { return m_viewer; }

    void UpdateDrawMode(int mode);

  protected:
    void BuildSceneGraph();

    void UpdateSceneGraph();

  private:
    int m_drawMode = 0;
    bool m_drawModeChanged = true;
    vsg::ref_ptr<vsg::Viewer> m_viewer;
    vsg::ref_ptr<vsg::Window> m_window;

    vsg::ref_ptr<vsg::LookAt> m_lookAt;
    vsg::ref_ptr<vsg::Camera> m_camera;
    vsg::ref_ptr<vsg::ProjectionMatrix> m_perspective;
    vsg::dvec3 m_up_vector;

    vsg::ref_ptr<vsg::Group> m_scenegraph;
    vsg::ref_ptr<vsg::Switch> m_dot_subgraph;
    vsg::ref_ptr<vsg::Switch> m_line_subgraph;
    vsg::ref_ptr<vsg::Switch> m_polygon_subgraph;

    vsg::ref_ptr<vsg::Builder> m_builderBodyDots;
    vsg::ref_ptr<vsg::Builder> m_builderWireFrame;
    vsg::ref_ptr<vsg::Builder> m_builderLighting;

    vsg::ref_ptr<vsg::CommandGraph> m_commandGraph;
    vsg::ref_ptr<vsg::RenderGraph> m_renderGraph;

    vsg::ref_ptr<chrono::vsg3d::VSGScreenshotHandler> m_screenshotHandler;

    ChSystem* m_system;
};

}  // namespace vsg3d
}  // namespace chrono

#endif
