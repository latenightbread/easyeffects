/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <adwaita.h>
#include "effects_base.hpp"
#include "ui_helpers.hpp"

namespace ui::stereo_tools_box {

G_BEGIN_DECLS

#define EE_TYPE_STEREO_TOOLS_BOX (stereo_tools_box_get_type())

G_DECLARE_FINAL_TYPE(StereoToolsBox, stereo_tools_box, EE, STEREO_TOOLS_BOX, GtkBox)

G_END_DECLS

auto create() -> StereoToolsBox*;

void setup(StereoToolsBox* self, std::shared_ptr<StereoTools> stereo_tools, const std::string& schema_path);

}  // namespace ui::stereo_tools_box
