/*****************************************************************************/

/*
 *      app.c  --  Configuration Application.
 *
 *      Copyright (C) 2000
 *        Thomas Sailer (sailer@ife.ee.ethz.ch)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*****************************************************************************/

#define _GNU_SOURCE
#define _REENTRANT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* AIX requires this to be the first thing in the file.  */
#ifndef __GNUC__
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

#include "configapp.h"

#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "callbacks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------- */

#ifdef WIN32

/* free result with g_free */

static gchar *strtogtk(const char *in)
{
        WCHAR *wch, *wp;
        GdkWChar *gch, *gp;
        unsigned int i, len;
        
        if (!(len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, in, -1, NULL, 0)))
                return NULL;
        wch = wp = alloca(sizeof(wch[0])*len);
        gch = gp = alloca(sizeof(gch[0])*(len+1));
        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, in, -1, wch, len))
                return NULL;
        for (i = 0, wp = wch, gp = gch; i < len && *wp; i++, wp++, gp++)
                *gp = *wp;
        *gp = 0;
        return gdk_wcstombs(gch);
}

static char *gtktostr(const gchar *in)
{
        union {
                GdkWChar g[4096];
                WCHAR w[4096];
        } u;
        GdkWChar *gp;
        WCHAR *wp;
        gint len;
        unsigned int i;
        int len2;
        char *ret;

        if ((len = gdk_mbstowcs(u.g, in, sizeof(u.g)/sizeof(u.g[0]))) == -1)
                return NULL;
        for (wp = u.w, gp = u.g, i = 0; i < len; i++, gp++, wp++)
                *wp = *gp;
        if (!(len2 = WideCharToMultiByte(CP_ACP, 0, u.w, len, NULL, 0, NULL, NULL)))
                return NULL;
        ret = g_malloc(len2+1);
        if (!ret)
                return NULL;
        WideCharToMultiByte(CP_ACP, 0, u.w, len, ret, len2, NULL, NULL);
        ret[len2] = 0;
        return ret;
}

#endif /* WIN32 */

/* ---------------------------------------------------------------------- */

void on_configtree_selection_changed(GtkTree *treex, gpointer user_data)
{
	GtkWidget *tree;
	GList *sel;
	const char *cfgname = NULL, *chname = NULL;

	tree = gtk_object_get_data(GTK_OBJECT(mainwindow), "configtree");
	sel = GTK_TREE_SELECTION(tree);
	if (sel) {
		cfgname = gtk_object_get_data(GTK_OBJECT(sel->data), "configname");
		chname = gtk_object_get_data(GTK_OBJECT(sel->data), "channame");
	}
	if (cfgname && chname) {
		gtk_widget_show(gtk_object_get_data(GTK_OBJECT(mainwindow), "newchannel"));
		gtk_widget_show(gtk_object_get_data(GTK_OBJECT(mainwindow), "deleteconfiguration"));
		gtk_widget_show(gtk_object_get_data(GTK_OBJECT(mainwindow), "deletechannel"));
	} else if (cfgname) {
		gtk_widget_show(gtk_object_get_data(GTK_OBJECT(mainwindow), "newchannel"));
		gtk_widget_show(gtk_object_get_data(GTK_OBJECT(mainwindow), "deleteconfiguration"));
		gtk_widget_hide(gtk_object_get_data(GTK_OBJECT(mainwindow), "deletechannel"));
	} else {
		gtk_widget_hide(gtk_object_get_data(GTK_OBJECT(mainwindow), "newchannel"));
		gtk_widget_hide(gtk_object_get_data(GTK_OBJECT(mainwindow), "deleteconfiguration"));
		gtk_widget_hide(gtk_object_get_data(GTK_OBJECT(mainwindow), "deletechannel"));
	}
	g_print("selection: cfg: %s  chan: %s\n", cfgname ?: "-", chname ?: "-");
}

static GtkWidget *create_notebookhead(GList *combo_items)
{
	GtkWidget *vbox, *hbox, *label, *combo, *combo_entry, *hsep;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	label = gtk_label_new(_("Mode"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 5);
	gtk_misc_set_padding(GTK_MISC(label), 7, 7);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	combo = gtk_combo_new();
	gtk_widget_show(combo);
	gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, TRUE, 0);
	gtk_combo_set_value_in_list(GTK_COMBO(combo), TRUE, FALSE);
	gtk_combo_set_popdown_strings(GTK_COMBO(combo), combo_items);
	combo_entry = GTK_COMBO(combo)->entry;
	gtk_widget_show(combo_entry);
	gtk_entry_set_editable(GTK_ENTRY(combo_entry), FALSE);
	hsep = gtk_hseparator_new();
	gtk_widget_show(hsep);
	gtk_box_pack_start(GTK_BOX(vbox), hsep, FALSE, TRUE, 3);
	gtk_object_set_data(GTK_OBJECT(vbox), "combo", combo);
	return vbox;
}

static GtkWidget *create_paramwidget(const struct modemparams *par, const char *cfgname, const char *chname, const char *typname)
{
	const struct modemparams *par2 = par;
	unsigned int parcnt = 0, i, j;
	GtkWidget *table, *w1, *w2;
	GtkObject *o1;
	GtkTooltips *tooltips;
	GList *list;
        char buf[128];
	const char *value, * const *cp;
	double nval;
#ifdef WIN32
        gchar *valueg;
        gchar *combov[8];
#endif
        
	if (!par)
		return gtk_vbox_new(FALSE, 0);
	while (par2->name) {
		par2++;
		parcnt++;
	}
	table = gtk_table_new(parcnt, 2, FALSE);
	gtk_widget_show(table);
	tooltips = gtk_tooltips_new();
	for (par2 = par, i = 0; i < parcnt; i++, par2++) {
		w1 = gtk_label_new(par2->label);
		gtk_widget_show(w1);
		gtk_table_attach(GTK_TABLE(table), w1, 0, 1, i, i+1, 
				 (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions) 0, 5, 5);
		gtk_misc_set_alignment(GTK_MISC(w1), 0, 0.5);
		gtk_label_set_justify(GTK_LABEL(w1), GTK_JUSTIFY_LEFT);
		if (xml_getprop(cfgname, chname, typname, par2->name, buf, sizeof(buf)) > 0)
                        value = buf;
                else
			value = par2->dflt;
		switch (par2->type) {
		case MODEMPAR_STRING:
			w1 = gtk_entry_new();
#ifdef WIN32
                        valueg = strtogtk(value);
			gtk_entry_set_text(GTK_ENTRY(w1), valueg ?: "(null)");
                        g_free(valueg);
#else
			gtk_entry_set_text(GTK_ENTRY(w1), value);
#endif
			break;

		case MODEMPAR_COMBO:
			w1 = gtk_combo_new();
#ifdef WIN32
			list = NULL;
			for (cp = par2->u.c.combostr, j = 0; *cp && j < 8; j++, cp++) {
                                combov[j] = strtogtk(*cp);
				list = g_list_append(list, combov[j] ?: "(null)");
                        }
			gtk_combo_set_popdown_strings(GTK_COMBO(w1), list);
			g_list_free(list);
                        for (; j > 0; j--)
                                g_free(combov[j-1]);
			w2 = GTK_COMBO(w1)->entry;
			gtk_widget_show(w2);
                        valueg = strtogtk(value);
			gtk_entry_set_text(GTK_ENTRY(w2), valueg ?: "(null)");
                        g_free(valueg);
#else
			list = NULL;
			for (cp = par2->u.c.combostr, j = 0; *cp && j < 8; j++, cp++)
				list = g_list_append(list, (void *)(*cp));
			gtk_combo_set_popdown_strings(GTK_COMBO(w1), list);
			g_list_free(list);
			w2 = GTK_COMBO(w1)->entry;
			gtk_widget_show(w2);
			gtk_entry_set_text(GTK_ENTRY(w2), value);
#endif
			gtk_combo_set_value_in_list(GTK_COMBO(w1), FALSE, FALSE);
			break;

		case MODEMPAR_NUMERIC:
			nval = strtod(value, NULL);
			if (nval < par2->u.n.min)
				nval = par2->u.n.min;
			if (nval > par2->u.n.max)
				nval = par2->u.n.max;
			o1 = gtk_adjustment_new(nval, par2->u.n.min, par2->u.n.max, par2->u.n.step, 
						par2->u.n.pagestep, par2->u.n.pagestep);
			w1 = gtk_spin_button_new(GTK_ADJUSTMENT(o1), par2->u.n.step, 0);
			break;

		case MODEMPAR_CHECKBUTTON:
			w1 = gtk_check_button_new();
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w1), (*value == '0') ? FALSE : TRUE);
			break;

		default:
			continue;
		}
		gtk_widget_show(w1);
		gtk_object_set_data(GTK_OBJECT(table), par2->name, w1);
		gtk_table_attach(GTK_TABLE(table), w1, 1, 2, i, i+1, 
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions) 0, 5, 5);
		if (par2->tooltip)
			gtk_tooltips_set_tip(tooltips, w1, par2->tooltip, NULL);
	}
	return table;
}

static void update_paramwidget(GtkWidget *table, const struct modemparams *par, const char *cfgname, const char *chname, const char *typname)
{
	const struct modemparams *par2 = par;
	GtkWidget *w1, *w2;
	char buf[256];
#ifdef WIN32
        char *txt;
#endif
        
	if (!par)
		return;
	for (par2 = par; par2->name; par2++) {
		w1 = gtk_object_get_data(GTK_OBJECT(table), par2->name);
		if (!w1)
			continue;
		switch (par2->type) {
		case MODEMPAR_STRING:
#ifdef WIN32
                        txt = gtktostr(gtk_entry_get_text(GTK_ENTRY(w1)));
			xml_setprop(cfgname, chname, typname, par2->name, txt ?: "(null)");
                        g_free(txt);
#else
			xml_setprop(cfgname, chname, typname, par2->name, gtk_entry_get_text(GTK_ENTRY(w1)));
#endif
			break;

		case MODEMPAR_COMBO:
			w2 = GTK_COMBO(w1)->entry;
#ifdef WIN32
                        txt = gtktostr(gtk_entry_get_text(GTK_ENTRY(w2)));
			xml_setprop(cfgname, chname, typname, par2->name, txt ?: "(null)");
                        g_free(txt);
#else
			xml_setprop(cfgname, chname, typname, par2->name, gtk_entry_get_text(GTK_ENTRY(w2)));
#endif
			break;

		case MODEMPAR_NUMERIC:
			sprintf(buf, "%g", gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(w1)));
			xml_setprop(cfgname, chname, typname, par2->name, buf);
			break;

		case MODEMPAR_CHECKBUTTON:
			xml_setprop(cfgname, chname, typname, par2->name, 
				    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w1)) ? "1" : "0");
			break;

		default:
			continue;
		}
	}
}

static void on_iotypecombochg_changed(GtkEditable *editable, gpointer user_data);

static void cfg_select(const char *cfgname, const char *chname)
{
	GtkWidget *notebook, *w1, *w2, *combo, *combo_entry;
	struct modemparams *ioparams = ioparams_soundcard;
	GList *ilist;
	char buf[128];
	unsigned int i;

	g_print("config_select: cfg: %s  chan: %s\n", cfgname ?: "-", chname ?: "-");

	notebook = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "confignotebook"));
	/* compute audio IO types */
	ilist = NULL;
	if (xml_getprop(cfgname, NULL, "audio", ioparam_type[0].name, buf, sizeof(buf)) <= 0)
                buf[0] = 0;
	if (!strcmp(buf, ioparam_type[0].u.c.combostr[1]))
		ioparams = ioparams_filein;
	else if (!strcmp(buf, ioparam_type[0].u.c.combostr[2]))
		ioparams = ioparams_sim;
        else {
		ioparams = ioparams_soundcard;
		strncpy(buf, ioparam_type[0].u.c.combostr[0], sizeof(buf));
	}
	for (i = 0; i < 8; i++)
		if (ioparam_type[0].u.c.combostr[i])
			ilist = g_list_append(ilist, (void *)ioparam_type[0].u.c.combostr[i]);
	w1 = create_notebookhead(ilist);
	gtk_object_set_data(GTK_OBJECT(w1), "cfgname", (void *)cfgname);
	gtk_object_set_data(GTK_OBJECT(w1), "chname", (void *)chname);
	g_list_free(ilist);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w1), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	gtk_entry_set_text(GTK_ENTRY(combo_entry), buf);
	/* create rest of notebook */
	w2 = create_paramwidget(ioparams, cfgname, NULL, "audio");
	gtk_box_pack_start(GTK_BOX(w1), w2, TRUE, TRUE, 10);
	gtk_object_set_data(GTK_OBJECT(w1), "audio", w2);
	gtk_object_set_data(GTK_OBJECT(w1), "audioparams", ioparams);
	w2 = gtk_hseparator_new();
	gtk_widget_show(w2);
	gtk_box_pack_start(GTK_BOX(w1), w2, FALSE, TRUE, 0);
	w2 = create_paramwidget(pttparams, cfgname, NULL, "ptt");
	gtk_box_pack_start(GTK_BOX(w1), w2, TRUE, TRUE, 10);
	gtk_object_set_data(GTK_OBJECT(w1), "ptt", w2);
	w2 = gtk_hseparator_new();
	gtk_widget_show(w2);
	gtk_box_pack_start(GTK_BOX(w1), w2, FALSE, TRUE, 0);

	w2 = gtk_label_new(_("IO"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), w1, w2);
	/* connect change signal */
	gtk_signal_connect(GTK_OBJECT(combo_entry), "changed",
			   GTK_SIGNAL_FUNC(on_iotypecombochg_changed), NULL);

	/* second page contains channel access parameters */
	w1 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(w1);
	gtk_object_set_data(GTK_OBJECT(w1), "cfgname", (void *)cfgname);
	gtk_object_set_data(GTK_OBJECT(w1), "chname", (void *)chname);
	w2 = create_paramwidget(chaccparams_x, cfgname, NULL, "chaccess");
	gtk_box_pack_start(GTK_BOX(w1), w2, TRUE, TRUE, 10);
	gtk_object_set_data(GTK_OBJECT(w1), "chacc", w2);

	w2 = gtk_label_new(_("Channel Access"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), w1, w2);
}

static void cfg_deselect(const char *cfgname, const char *chname)
{
	GtkWidget *notebook, *w1, *combo, *combo_entry;
	struct modemparams *ioparams;

	g_print("config_deselect: cfg: %s  chan: %s\n", cfgname ?: "-", chname ?: "-");

	notebook = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "confignotebook"));
	w1 = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 1);
	if (w1) {
		update_paramwidget(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w1), "chacc")), chaccparams_x, cfgname, NULL, "chaccess");
		gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), 1);
	}
	w1 = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 0);
	if (!w1)
		return;
	/* update type */
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w1), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	xml_setprop(cfgname, NULL, "audio", ioparam_type[0].name, gtk_entry_get_text(GTK_ENTRY(combo_entry)));
	ioparams = (struct modemparams *)gtk_object_get_data(GTK_OBJECT(w1), "audioparams");
	update_paramwidget(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w1), "audio")), ioparams, cfgname, NULL, "audio");
	update_paramwidget(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w1), "ptt")), pttparams, cfgname, NULL, "ptt");
	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), 0);
}

static guint iotypecombochg = 0;

static gint do_iotypecombochg_change(gpointer user_data)
{
	GtkWidget *notebook, *w;
	const char *cfgname, *chname;
	gint nbcurpage;

	iotypecombochg = 0;
	/* recreate notebook widgets */
	notebook = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "confignotebook"));
	/* find config strings */
	w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 0);
	cfgname = gtk_object_get_data(GTK_OBJECT(w), "cfgname");
	chname = gtk_object_get_data(GTK_OBJECT(w), "chname");

	g_print("on_notebookcombo_changed: cfg: %s  chan: %s\n", cfgname ?: "-", chname ?: "-");

	nbcurpage = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	cfg_deselect(cfgname, chname);
	g_print("Recreating menus\n");
	cfg_select(cfgname, chname);
	gtk_notebook_set_page(GTK_NOTEBOOK(notebook), nbcurpage);
	g_print("Returning\n");
	return FALSE;
}

static void on_iotypecombochg_changed(GtkEditable *editable, gpointer user_data)
{
	if (!iotypecombochg)
		iotypecombochg = gtk_idle_add_priority(G_PRIORITY_HIGH, do_iotypecombochg_change, NULL);
}


static void on_config_select(GtkWidget *item, gpointer data)
{
	const char *cfgname = NULL, *chname = NULL;

	cfgname = gtk_object_get_data(GTK_OBJECT(item), "configname");
	chname = gtk_object_get_data(GTK_OBJECT(item), "channame");
	cfg_select(cfgname, chname);
}

static void on_config_deselect(GtkWidget *item, gpointer data)
{
	const char *cfgname = NULL, *chname = NULL;

	cfgname = gtk_object_get_data(GTK_OBJECT(item), "configname");
	chname = gtk_object_get_data(GTK_OBJECT(item), "channame");
	cfg_deselect(cfgname, chname);
}

struct packetio {
	struct packetio *next;
	const char *name;
	const struct modemparams *params;
};

#ifndef WIN32
#ifdef HAVE_MKISS

static struct packetio pktkiss = { NULL, "KISS", pktkissparams_x };
static struct packetio packetchain = { &pktkiss, "MKISS", pktmkissparams_x };

#else /* HAVE_MKISS */

static struct packetio packetchain = { NULL, "KISS", pktkissparams_x };

#endif /* HAVE_MKISS */
#endif /* WIN32 */


static void on_notebookcombo_changed(GtkEditable *editable, gpointer user_data);

static void make_notebook_menus(const char *cfgname, const char *chname)
{
	GtkWidget *notebook, *w1, *w2, *combo, *combo_entry;
	GList *ilist;
	struct modulator *modch = &modchain_x, *modch1 = &modchain_x;
	struct demodulator *demodch = &demodchain_x, *demodch1 = &demodchain_x;
#ifndef WIN32
	struct packetio *pktch = &packetchain, *pktch1 = &packetchain;
#endif /* WIN32 */
	char mode[128];

	notebook = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "confignotebook"));
	/* Modulator Tab */
	ilist = NULL;
	if (xml_getprop(cfgname, chname, "mod", "mode", mode, sizeof(mode)) <= 0)
                mode[0] = 0;
	for (; modch; modch = modch->next) {
		ilist = g_list_append(ilist, (void *)modch->name);
		if (!strcmp(mode, modch->name))
			modch1 = modch;
	}
	w1 = create_notebookhead(ilist);
	g_list_free(ilist);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w1), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	gtk_entry_set_text(GTK_ENTRY(combo_entry), modch1->name);
	w2 = create_paramwidget(modch1->params, cfgname, chname, "mod");
	gtk_object_set_data(GTK_OBJECT(w1), "cfgname", (void *)cfgname);
	gtk_object_set_data(GTK_OBJECT(w1), "chname", (void *)chname);
	gtk_object_set_data(GTK_OBJECT(w1), "par", (void *)modch1->params);
	gtk_object_set_data(GTK_OBJECT(w1), "table", w2);
	gtk_box_pack_start(GTK_BOX(w1), w2, TRUE, TRUE, 1);
	w2 = gtk_label_new(_("Modulator"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), w1, w2);
	gtk_signal_connect(GTK_OBJECT(combo_entry), "changed",
			   GTK_SIGNAL_FUNC(on_notebookcombo_changed), NULL);
	/* Demodulator Tab */
	ilist = NULL;
	if (xml_getprop(cfgname, chname, "demod", "mode", mode, sizeof(mode)) <= 0)
                mode[0] = 0;
	for (; demodch; demodch = demodch->next) {
		ilist = g_list_append(ilist, (void *)demodch->name);
		if (!strcmp(mode, demodch->name))
			demodch1 = demodch;
	}
	w1 = create_notebookhead(ilist);
	g_list_free(ilist);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w1), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	gtk_entry_set_text(GTK_ENTRY(combo_entry), demodch1->name);
	w2 = create_paramwidget(demodch1->params, cfgname, chname, "demod");
	gtk_object_set_data(GTK_OBJECT(w1), "cfgname", (void *)cfgname);
	gtk_object_set_data(GTK_OBJECT(w1), "chname", (void *)chname);
	gtk_object_set_data(GTK_OBJECT(w1), "par", (void *)demodch1->params);
	gtk_object_set_data(GTK_OBJECT(w1), "table", w2);
	gtk_box_pack_start(GTK_BOX(w1), w2, TRUE, TRUE, 1);
	w2 = gtk_label_new(_("Demodulator"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), w1, w2);
	gtk_signal_connect(GTK_OBJECT(combo_entry), "changed",
			   GTK_SIGNAL_FUNC(on_notebookcombo_changed), NULL);
#ifndef WIN32
	/* Packet IO Tab */
	ilist = NULL;
	if (xml_getprop(cfgname, chname, "pkt", "mode", mode, sizeof(mode)) <= 0)
                mode[0] = 0;
	for (; pktch; pktch = pktch->next) {
		ilist = g_list_append(ilist, (void *)pktch->name);
		if (!strcmp(mode, pktch->name))
			pktch1 = pktch;
	}
	w1 = create_notebookhead(ilist);
	g_list_free(ilist);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w1), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	gtk_entry_set_text(GTK_ENTRY(combo_entry), pktch1->name);
	w2 = create_paramwidget(pktch1->params, cfgname, chname, "pkt");
	gtk_object_set_data(GTK_OBJECT(w1), "cfgname", (void *)cfgname);
	gtk_object_set_data(GTK_OBJECT(w1), "chname", (void *)chname);
	gtk_object_set_data(GTK_OBJECT(w1), "par", (void *)pktch1->params);
	gtk_object_set_data(GTK_OBJECT(w1), "table", w2);
	gtk_box_pack_start(GTK_BOX(w1), w2, TRUE, TRUE, 1);
	w2 = gtk_label_new(_("Packet IO"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), w1, w2);
	gtk_signal_connect(GTK_OBJECT(combo_entry), "changed",
			   GTK_SIGNAL_FUNC(on_notebookcombo_changed), NULL);
#endif /* WIN32 */
}

static void destroy_notebook_menus(void)
{
	GtkWidget *w, *notebook, *combo, *combo_entry;
	const char *cfgname, *chname;
	struct modemparams *par;
	
	notebook = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "confignotebook"));
	/* find config strings */
	w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 0);
	cfgname = gtk_object_get_data(GTK_OBJECT(w), "cfgname");
	chname = gtk_object_get_data(GTK_OBJECT(w), "chname");
	if (!cfgname || !chname) {
		g_printerr("destroy_notebook_menus: cfgname or chname NULL!\n");
		return;
	}
	/* update modulator */
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	xml_setprop(cfgname, chname, "mod", "mode", gtk_entry_get_text(GTK_ENTRY(combo_entry)));
g_print("Modulator mode: %s\n", gtk_entry_get_text(GTK_ENTRY(combo_entry)));
	par = gtk_object_get_data(GTK_OBJECT(w), "par");
	update_paramwidget(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "table")), par, cfgname, chname, "mod");
	/* update demodulator */
	w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 1);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	xml_setprop(cfgname, chname, "demod", "mode", gtk_entry_get_text(GTK_ENTRY(combo_entry)));
g_print("Demodulator mode: %s\n", gtk_entry_get_text(GTK_ENTRY(combo_entry)));
	par = gtk_object_get_data(GTK_OBJECT(w), "par");
	update_paramwidget(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "table")), par, cfgname, chname, "demod");
	/* update KISS stuff */
#ifndef WIN32
	w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 2);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	xml_setprop(cfgname, chname, "pkt", "mode", gtk_entry_get_text(GTK_ENTRY(combo_entry)));
g_print("Packet IO mode: %s\n", gtk_entry_get_text(GTK_ENTRY(combo_entry)));
	par = gtk_object_get_data(GTK_OBJECT(w), "par");
	update_paramwidget(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "table")), par, cfgname, chname, "pkt");
#endif /* WIN32 */
	/* delete pages */
#ifndef WIN32
	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), 2);
#endif /* WIN32 */
	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), 1);
	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), 0);
}

static guint notebookcombochg = 0;

static gint do_notebookcombo_change(gpointer user_data)
{
	GtkWidget *notebook, *w, *combo, *combo_entry;
	const char *cfgname, *chname, *text2;
        char text1[128];
	gint nbcurpage, changed = 0;

	notebookcombochg = 0;
	/* recreate notebook widgets */
	notebook = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "confignotebook"));
	/* find config strings */
	w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 0);
	cfgname = gtk_object_get_data(GTK_OBJECT(w), "cfgname");
	chname = gtk_object_get_data(GTK_OBJECT(w), "chname");

	g_print("on_notebookcombo_changed: cfg: %s  chan: %s\n", cfgname ?: "-", chname ?: "-");
#if 0
	/* check if something changed */
	w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 0);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	xml_getprop(cfgname, chname, "mod", "mode", text1, sizeof(text1));
	text2 = gtk_entry_get_text(GTK_ENTRY(combo_entry));
	if (!text2 || strcmp(text1, text2))
		changed = 1;
	g_printerr("Text1 %s Text2 %s chg %d\n", text1, text2, changed);
	w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 1);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	xml_getprop(cfgname, chname, "demod", "mode", text1, sizeof(text1));
	text2 = gtk_entry_get_text(GTK_ENTRY(combo_entry));
	if (!text2 || strcmp(text1, text2))
		changed = 1;
	g_printerr("Text1 %s Text2 %s chg %d\n", text1, text2, changed);
#ifndef WIN32
	w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 2);
	combo = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(w), "combo"));
	combo_entry = GTK_COMBO(combo)->entry;
	xml_getprop(cfgname, chname, "pkt", "mode", text1, sizeof(text1));
	text2 = gtk_entry_get_text(GTK_ENTRY(combo_entry));
	if (!text2 || strcmp(text1, text2))
		changed = 1;
	g_printerr("Text1 %s Text2 %s chg %d\n", text1, text2, changed);
#endif /* WIN32 */
	if (!changed)
		return FALSE;
#endif
	nbcurpage = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	destroy_notebook_menus();
	g_print("Recreating menus\n");
	make_notebook_menus(cfgname, chname);
	gtk_notebook_set_page(GTK_NOTEBOOK(notebook), nbcurpage);
	g_print("Returning\n");
	return FALSE;
}

static void on_notebookcombo_changed(GtkEditable *editable, gpointer user_data)
{
	/*GtkWidget *vbox = GTK_WIDGET(user_data);*/

	if (!notebookcombochg)
		notebookcombochg = gtk_idle_add_priority(G_PRIORITY_HIGH, do_notebookcombo_change, NULL);
}

static void on_channel_select(GtkWidget *item, gpointer data)
{
	const char *cfgname = NULL, *chname = NULL;

	cfgname = gtk_object_get_data(GTK_OBJECT(item), "configname");
	chname = gtk_object_get_data(GTK_OBJECT(item), "channame");

	g_print("channel_select: cfg: %s  chan: %s\n", cfgname ?: "-", chname ?: "-");

	make_notebook_menus(cfgname, chname);
	gtk_widget_show(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "diagnostics")));
}

static void on_channel_deselect(GtkWidget *item, gpointer data)
{
	const char *cfgname = NULL, *chname = NULL;

	if (notebookcombochg)
		gtk_idle_remove(notebookcombochg);
	notebookcombochg = 0;

	cfgname = gtk_object_get_data(GTK_OBJECT(item), "configname");
	chname = gtk_object_get_data(GTK_OBJECT(item), "channame");

	g_print("channel_deselect: cfg: %s  chan: %s\n", cfgname ?: "-", chname ?: "-");

	destroy_notebook_menus();
	gtk_widget_hide(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "diagnostics")));
	diag_stop();
}

/* ---------------------------------------------------------------------- */

static void dounselect(void)
{
	GtkWidget *tree;
	GList *chld;

	tree = gtk_object_get_data(GTK_OBJECT(mainwindow), "configtree");
	chld = GTK_TREE_SELECTION(tree);
	if (chld)
		gtk_tree_unselect_child(GTK_TREE(GTK_WIDGET(chld->data)->parent), GTK_WIDGET(chld->data));
}

void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	dounselect();
        gtk_main_quit();
}

gboolean on_mainwindow_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	g_print("destroy event\n");
	dounselect();
        gtk_main_quit();
	return TRUE;
}

gboolean on_mainwindow_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	g_print("delete event\n");
	dounselect();
        gtk_main_quit();
	return FALSE;
}

static void on_aboutok_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(GTK_WIDGET(user_data));
	gtk_widget_destroy(GTK_WIDGET(user_data));
}

void on_about_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dlg = create_aboutwindow();

	gtk_signal_connect(GTK_OBJECT(gtk_object_get_data(GTK_OBJECT(dlg), "aboutok")), 
			   "clicked", GTK_SIGNAL_FUNC(on_aboutok_clicked), dlg);
	gtk_widget_show(dlg);
}


/* ---------------------------------------------------------------------- */

static GtkWidget *findconfigitem(const gchar *name)
{
	GtkWidget *tree;
	GList *chld;
	const gchar *nm;

	tree = gtk_object_get_data(GTK_OBJECT(mainwindow), "configtree");
	chld = GTK_TREE(tree)->children;
	for (; chld; chld = chld->next) {
		if (!(nm = gtk_object_get_data(GTK_OBJECT(chld->data), "configname")))
			continue;
		if (strcmp(nm, name))
			continue;
		return GTK_WIDGET(chld->data);
	}
	return NULL;
}

static GtkWidget *findchannelitem(GtkWidget *cfgitem, const gchar *name)
{
	GtkWidget *tree;
	GList *chld;
	const gchar *nm;

	tree = GTK_TREE_ITEM(cfgitem)->subtree;
	chld = GTK_TREE(tree)->children;
	for (; chld; chld = chld->next) {
		if (!(nm = gtk_object_get_data(GTK_OBJECT(chld->data), "channame")))
			continue;
		if (strcmp(nm, name))
			continue;
		return GTK_WIDGET(chld->data);
	}
	return NULL;
}

GtkWidget *new_configuration(const gchar *name2)
{
	GtkWidget *item, *tree;
	char *name;

	if ((item = findconfigitem(name2)))
		return item;
	name = strdup(name2);
	tree = gtk_object_get_data(GTK_OBJECT(mainwindow), "configtree");
        item = gtk_tree_item_new_with_label(name);
	gtk_signal_connect_after(GTK_OBJECT(item), "select", GTK_SIGNAL_FUNC(on_config_select), item);
	gtk_signal_connect_after(GTK_OBJECT(item), "deselect", GTK_SIGNAL_FUNC(on_config_deselect), item);
	gtk_tree_append(GTK_TREE(tree), item);
	gtk_widget_show(item);
	gtk_object_set_data(GTK_OBJECT(item), "configitem", item);
	gtk_object_set_data(GTK_OBJECT(item), "parenttree", tree);
	gtk_object_set_data(GTK_OBJECT(item), "configname", (void *)name);
	gtk_object_set_data(GTK_OBJECT(item), "channame", NULL);
	return item;
}

GtkWidget *new_channel(const gchar *cfgname, const gchar *name)
{
	GtkWidget *item, *tree, *cfgitem;

	cfgitem = findconfigitem(cfgname);
	if (!cfgitem) {
		g_printerr("Could not find configuration \"%s\"\n", cfgname);
		return NULL;
	}
	tree = GTK_TREE_ITEM_SUBTREE(cfgitem);
	if (!tree) {
		tree = gtk_tree_new();
		gtk_tree_item_set_subtree(GTK_TREE_ITEM(cfgitem), tree);
	}
	item = gtk_tree_item_new_with_label("");
	gtk_signal_connect_after(GTK_OBJECT(item), "select", GTK_SIGNAL_FUNC(on_channel_select), item);
	gtk_signal_connect_after(GTK_OBJECT(item), "deselect", GTK_SIGNAL_FUNC(on_channel_deselect), item);
	gtk_tree_append(GTK_TREE(tree), item);
	gtk_tree_item_expand(GTK_TREE_ITEM(cfgitem));
	gtk_widget_show(item);
	gtk_object_set_data(GTK_OBJECT(item), "configitem", cfgitem);
	gtk_object_set_data(GTK_OBJECT(item), "parenttree", tree);
	gtk_object_set_data(GTK_OBJECT(item), "configname", gtk_object_get_data(GTK_OBJECT(cfgitem), "configname"));
	gtk_object_set_data(GTK_OBJECT(item), "channame", (void *)strdup(name));
	return item;
}

static void renumber_onecfg(GtkWidget *tree)
{
	unsigned int cnt = 0;
	gchar buf[64];
	GtkWidget *label;
	GList *chld;

	if (!tree)
		return;
	chld = GTK_TREE(tree)->children;
	for (; chld; chld = chld->next) {
		label = GTK_BIN(chld->data)->child;
		sprintf(buf, "Channel %u", cnt++);
		gtk_label_set_text(GTK_LABEL(label), buf);
	}
}

void renumber_channels(void)
{
	GtkWidget *tree;
	GList *chld;

	tree = gtk_object_get_data(GTK_OBJECT(mainwindow), "configtree");
	chld = GTK_TREE(tree)->children;
	for (; chld; chld = chld->next) {
		renumber_onecfg(GTK_TREE_ITEM(chld->data)->subtree);
	}
}

/*    gtk_container_remove */
/* ---------------------------------------------------------------------- */

static void on_errorok_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(GTK_WIDGET(user_data));
	gtk_widget_destroy(GTK_WIDGET(user_data));
}

void error_dialog(const gchar *text)
{
	GtkWidget *dlg = create_errordialog();

	gtk_signal_connect(GTK_OBJECT(gtk_object_get_data(GTK_OBJECT(dlg), "errorok")), 
			   "clicked", GTK_SIGNAL_FUNC(on_errorok_clicked), dlg);
	gtk_label_set_text(GTK_LABEL(gtk_object_get_data(GTK_OBJECT(dlg), "errorlabel")), text);
	gtk_widget_show(dlg);
}

/* ---------------------------------------------------------------------- */

static void on_newconfigok_clicked(GtkButton *button, gpointer user_data)
{
	gchar *text = gtk_entry_get_text(GTK_ENTRY(gtk_object_get_data(GTK_OBJECT(user_data), "newconfigentry")));
	int ret;
	char buf[128];
	
	gtk_widget_hide(GTK_WIDGET(user_data));
	if (text[0]) {
		ret = xml_newconfig(text);
		if (ret) {
			snprintf(buf, sizeof(buf), "Duplicate name: \"%s\"\n", text);
			error_dialog(buf);
		} else {
			new_configuration(text);
		}
	}
	gtk_widget_destroy(GTK_WIDGET(user_data));
}

static void on_newconfigcancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_hide(GTK_WIDGET(user_data));
	gtk_widget_destroy(GTK_WIDGET(user_data));
}

void on_newconfiguration_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dlg = create_newconfigwindow();

	gtk_signal_connect(GTK_OBJECT(gtk_object_get_data(GTK_OBJECT(dlg), "newconfigok")), 
			   "clicked", GTK_SIGNAL_FUNC(on_newconfigok_clicked), dlg);
	gtk_signal_connect(GTK_OBJECT(gtk_object_get_data(GTK_OBJECT(dlg), "newconfigcancel")), 
			   "clicked", GTK_SIGNAL_FUNC(on_newconfigcancel_clicked), dlg);
	gtk_widget_show(dlg);
}

/* ---------------------------------------------------------------------- */

void on_newchannel_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *tree;
	GList *sel;
	const char *cfgname = NULL, *chname = NULL;

	tree = gtk_object_get_data(GTK_OBJECT(mainwindow), "configtree");
	sel = GTK_TREE_SELECTION(tree);
	if (sel)
		cfgname = gtk_object_get_data(GTK_OBJECT(sel->data), "configname");
	if (!cfgname) {
		g_printerr("on_newchannel_activate: cfgname NULL\n");
		return;
	}
	chname = xml_newchannel(cfgname);
	if (!chname) {
		g_printerr("on_newchannel_activate: cannot create new channel\n");
		return;
	}
	new_channel(cfgname, chname);
	renumber_channels();
}


void on_deleteconfiguration_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *tree, *item;
	GList *sel;
	const char *cfgname = NULL;
	GList list = { NULL, NULL, NULL };

	tree = gtk_object_get_data(GTK_OBJECT(mainwindow), "configtree");
	sel = GTK_TREE_SELECTION(tree);
	if (sel)
		cfgname = gtk_object_get_data(GTK_OBJECT(sel->data), "configname");
	dounselect();
	if (!cfgname)
		return;
	item = findconfigitem(cfgname);
	if (xml_deleteconfig(cfgname))
		return;
	free((char *)cfgname);
	gtk_container_remove(GTK_CONTAINER(tree), item);
	//list.data = item;
	//gtk_tree_remove_items(GTK_TREE(tree), &list);
}


void on_deletechannel_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *tree, *cfgitem, *chitem;
	GList *sel;
	const char *cfgname = NULL, *chname = NULL;
	GList list = { NULL, NULL, NULL };

	tree = gtk_object_get_data(GTK_OBJECT(mainwindow), "configtree");
	sel = GTK_TREE_SELECTION(tree);
	if (sel) {
		cfgname = gtk_object_get_data(GTK_OBJECT(sel->data), "configname");
		chname = gtk_object_get_data(GTK_OBJECT(sel->data), "channame");
	}
	dounselect();
	if (!cfgname || !chname)
		return;
	cfgitem = findconfigitem(cfgname);
	chitem = findchannelitem(cfgitem, chname);
	g_print("delete channel cfg %s ch %s\n", cfgname, chname);
	if (xml_deletechannel(cfgname, chname))
		return;
	gtk_container_remove(GTK_CONTAINER(GTK_TREE_ITEM(cfgitem)->subtree), chitem);
        free((char *)chname);
	//list.data = chitem;
	//gtk_tree_remove_items(GTK_TREE(tree), &list);
	renumber_channels();
}
