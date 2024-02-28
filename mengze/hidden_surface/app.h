#pragma once

#include "imgui.h"

#include "core/application.h"
#include "rendering/render_layer.h"
#include "rendering/renderer.h"

#include "hidden_surface//scanline_zbuffer.h"
#include "hidden_surface/gui.h"
#include "hidden_surface/hierarchical_zbuffer.h"
#include "hidden_surface/zbuffer.h"


void hidden_surface_app_setup(mengze::Application &app);
