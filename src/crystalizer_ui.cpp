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

#include "crystalizer_ui.hpp"

namespace ui::crystalizer_box {

using namespace std::string_literals;

auto constexpr log_tag = "crystalizer_box: ";

constexpr uint nbands = 13U;

struct Data {
 public:
  ~Data() { util::debug(log_tag + "data struct destroyed"s); }

  std::shared_ptr<Crystalizer> crystalizer;

  std::vector<sigc::connection> connections;

  std::vector<gulong> gconnections;
};

struct _CrystalizerBox {
  GtkBox parent_instance;

  GtkScale *input_gain, *output_gain;

  GtkLevelBar *input_level_left, *input_level_right, *output_level_left, *output_level_right;

  GtkLabel *input_level_left_label, *input_level_right_label, *output_level_left_label, *output_level_right_label;

  GtkToggleButton* bypass;

  GtkBox* bands_box;

  GSettings* settings;

  Data* data;
};

G_DEFINE_TYPE(CrystalizerBox, crystalizer_box, GTK_TYPE_BOX)

void on_bypass(CrystalizerBox* self, GtkToggleButton* btn) {
  self->data->crystalizer->bypass = gtk_toggle_button_get_active(btn);
}

void on_reset(CrystalizerBox* self, GtkButton* btn) {
  gtk_toggle_button_set_active(self->bypass, 0);

  util::reset_all_keys(self->settings);
}

void build_bands(CrystalizerBox* self) {
  for (uint n = 0; n < nbands; n++) {
    auto builder = gtk_builder_new_from_resource("/com/github/wwmm/easyeffects/ui/crystalizer_band.ui");

    auto* band_box = GTK_BOX(gtk_builder_get_object(builder, "band_box"));

    auto* band_label = GTK_LABEL(gtk_builder_get_object(builder, "band_label"));

    auto* band_intensity = GTK_SCALE(gtk_builder_get_object(builder, "band_intensity"));

    auto* band_bypass = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "band_bypass"));
    auto* band_mute = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "band_mute"));

    prepare_scale<"">(band_intensity);

    const auto bandn = "band" + std::to_string(n);

    // connections

    g_settings_bind(self->settings, ("intensity-" + bandn).c_str(), gtk_range_get_adjustment(GTK_RANGE(band_intensity)),
                    "value", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind(self->settings, ("mute-" + bandn).c_str(), band_mute, "active", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(self->settings, ("bypass-" + bandn).c_str(), band_bypass, "active", G_SETTINGS_BIND_DEFAULT);

    switch (n) {
      case 0:
        gtk_label_set_text(band_label, "250 Hz");
        break;
      case 1:
        gtk_label_set_text(band_label, "750 Hz");
        break;
      case 2:
        gtk_label_set_text(band_label, "1.5 kHz");
        break;
      case 3:
        gtk_label_set_text(band_label, "2.5 kHz");
        break;
      case 4:
        gtk_label_set_text(band_label, "3.5 kHz");
        break;
      case 5:
        gtk_label_set_text(band_label, "4.5 kHz");
        break;
      case 6:
        gtk_label_set_text(band_label, "5.5 kHz");
        break;
      case 7:
        gtk_label_set_text(band_label, "6.5 kHz");
        break;
      case 8:
        gtk_label_set_text(band_label, "7.5 kHz");
        break;
      case 9:
        gtk_label_set_text(band_label, "8.5 kHz");
        break;
      case 10:
        gtk_label_set_text(band_label, "9.5 kHz");
        break;
      case 11:
        gtk_label_set_text(band_label, "12.5 kHz");
        break;
      case 12:
        gtk_label_set_text(band_label, "17.5 kHz");
        break;
    }

    gtk_box_append(self->bands_box, GTK_WIDGET(band_box));

    g_object_unref(builder);
  }
}

void setup(CrystalizerBox* self, std::shared_ptr<Crystalizer> crystalizer, const std::string& schema_path) {
  self->data->crystalizer = crystalizer;

  self->settings = g_settings_new_with_path("com.github.wwmm.easyeffects.crystalizer", schema_path.c_str());

  crystalizer->post_messages = true;
  crystalizer->bypass = false;

  build_bands(self);

  self->data->connections.push_back(crystalizer->input_level.connect([=](const float& left, const float& right) {
    update_level(self->input_level_left, self->input_level_left_label, self->input_level_right,
                 self->input_level_right_label, left, right);
  }));

  self->data->connections.push_back(crystalizer->output_level.connect([=](const float& left, const float& right) {
    update_level(self->output_level_left, self->output_level_left_label, self->output_level_right,
                 self->output_level_right_label, left, right);
  }));

  gsettings_bind_widgets<"input-gain", "output-gain">(self->settings, self->input_gain, self->output_gain);
}

void dispose(GObject* object) {
  auto* self = EE_CRYSTALIZER_BOX(object);

  self->data->crystalizer->bypass = false;

  for (auto& c : self->data->connections) {
    c.disconnect();
  }

  for (auto& handler_id : self->data->gconnections) {
    g_signal_handler_disconnect(self->settings, handler_id);
  }

  self->data->connections.clear();
  self->data->gconnections.clear();

  g_object_unref(self->settings);

  util::debug(log_tag + "disposed"s);

  G_OBJECT_CLASS(crystalizer_box_parent_class)->dispose(object);
}

void finalize(GObject* object) {
  auto* self = EE_CRYSTALIZER_BOX(object);

  delete self->data;

  util::debug(log_tag + "finalized"s);

  G_OBJECT_CLASS(crystalizer_box_parent_class)->finalize(object);
}

void crystalizer_box_class_init(CrystalizerBoxClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;
  object_class->finalize = finalize;

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/crystalizer.ui");

  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, input_gain);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, output_gain);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, input_level_left);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, input_level_right);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, output_level_left);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, output_level_right);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, input_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, input_level_right_label);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, output_level_left_label);
  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, output_level_right_label);

  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, bypass);

  gtk_widget_class_bind_template_child(widget_class, CrystalizerBox, bands_box);

  gtk_widget_class_bind_template_callback(widget_class, on_bypass);
  gtk_widget_class_bind_template_callback(widget_class, on_reset);
}

void crystalizer_box_init(CrystalizerBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->data = new Data();

  prepare_scales<"dB">(self->input_gain, self->output_gain);
}

auto create() -> CrystalizerBox* {
  return static_cast<CrystalizerBox*>(g_object_new(EE_TYPE_CRYSTALIZER_BOX, nullptr));
}

}  // namespace ui::crystalizer_box
