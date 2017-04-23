/*

  Copyright (C) 2017 Gonzalo José Carracedo Carballal

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program.  If not, see
  <http://www.gnu.org/licenses/>

*/

#ifndef _GUI_GUI_H
#define _GUI_GUI_H

#include <sigutils/sigutils.h>
#include <suscan.h>
#include <gtk/gtk.h>

#ifndef PKGDATADIR
#define PKGDATADIR "/usr"
#endif

struct suscan_gui_source_config {
  struct suscan_source *source;
  struct suscan_source_config *config;
  PTR_LIST(GtkWidget, widget);
  GtkGrid *grid;
};

enum suscan_gui_state {
  SUSCAN_GUI_STATE_STOPPED,
  SUSCAN_GUI_STATE_RUNNING,
  SUSCAN_GUI_STATE_STOPPING
};

struct suscan_gui {
  /* Widgets */
  GtkBuilder *builder;
  GtkWindow *main;
  GtkButton *toggleConnect;
  GtkButton *preferencesButton;
  GtkListStore *sourceListStore;
  GtkListStore *channelListStore;
  GtkDialog *settingsDialog;
  GtkComboBox *sourceCombo;
  GtkHeaderBar *headerBar;
  GtkMenuBar *menuBar;
  GtkLabel *freqLabels[10];
  GObject *sourceAlignment;

  GtkTreeViewColumn *centerFrequencyCol;
  GtkTreeViewColumn *snrCol;
  GtkTreeViewColumn *signalLevelCol;
  GtkTreeViewColumn *noiseLevelCol;
  GtkTreeViewColumn *bandwidthCol;

  GtkCellRendererText *centerFrequencyCellRenderer;
  GtkCellRendererText *snrCellRenderer;
  GtkCellRendererText *signalLevelCellRenderer;
  GtkCellRendererText *noiseLevelCellRenderer;
  GtkCellRendererText *bandwidthCellRenderer;


  GtkLevelBar *cpuLevelBar;
  GtkLabel *cpuLabel;

  struct suscan_gui_source_config *selected_config;

  /* GUI state */
  enum suscan_gui_state state;

  /* Analyzer integration */
  suscan_analyzer_t *analyzer;
  struct suscan_mq mq_out;
  GThread *async_thread;
};

struct suscan_gui *suscan_gui_new(int argc, char **argv);
SUBOOL suscan_gui_start(
    int argc,
    char **argv,
    struct suscan_source_config **config_list,
    unsigned int config_count);

void suscan_gui_msgbox(
    struct suscan_gui *gui,
    GtkMessageType type,
    const char *title,
    const char *fmt,
    ...);

struct suscan_gui_source_config *suscan_gui_get_selected_source(
    struct suscan_gui *gui);

SUBOOL suscan_gui_source_config_parse(struct suscan_gui_source_config *config);

void suscan_gui_set_config(
    struct suscan_gui *gui,
    struct suscan_gui_source_config *config);

void suscan_gui_update_state(
    struct suscan_gui *gui,
    enum suscan_gui_state state);

SUBOOL suscan_gui_connect(struct suscan_gui *gui);
void suscan_gui_disconnect(struct suscan_gui *gui);

#define suscan_error(gui, title, fmt, arg...) \
    suscan_gui_msgbox(gui, GTK_MESSAGE_ERROR, title, fmt, ##arg)

#define suscan_warning(gui, title, fmt, arg...) \
    suscan_gui_msgbox(gui, GTK_MESSAGE_WARNING, title, fmt, ##arg)

#endif /* _GUI_GUI_H */