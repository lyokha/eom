/* Eye Of Mate - Python Plugin
 *
 * Copyright (C) 2007 The Free Software Foundation
 *
 * Author: Lucas Rocha <lucasr@gnome.org>
 *
 * Based on gedit code (gedit/gedit-python-module.h) by:
 * 	- Raphael Slinckx <raphael@slinckx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include "eom-python-plugin.h"
#include "eom-plugin.h"
#include "eom-debug.h"

#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>
#include <string.h>

static GObjectClass *parent_class;

static PyObject *
call_python_method (EomPythonPlugin *plugin,
		    EomWindow       *window,
		    gchar             *method)
{
	PyObject *py_ret = NULL;

	g_return_val_if_fail (PyObject_HasAttrString (plugin->instance, method), NULL);

	if (window == NULL) {
		py_ret = PyObject_CallMethod (plugin->instance,
					      method,
					      NULL);
	} else {
		py_ret = PyObject_CallMethod (plugin->instance,
					      method,
					      "(N)",
					      pygobject_new (G_OBJECT (window)));
	}

	if (!py_ret)
		PyErr_Print ();

	return py_ret;
}

static gboolean
check_py_object_is_gtk_widget (PyObject *py_obj)
{
	static PyTypeObject *_PyGtkWidget_Type = NULL;

	if (_PyGtkWidget_Type == NULL) {
		PyObject *module;

	    	if ((module = PyImport_ImportModule ("gtk"))) {
			PyObject *moddict = PyModule_GetDict (module);
			_PyGtkWidget_Type = (PyTypeObject *) PyDict_GetItemString (moddict, "Widget");
	    	}

		if (_PyGtkWidget_Type == NULL) {
			PyErr_SetString(PyExc_TypeError, "could not find Python gtk widget type");
			PyErr_Print();

			return FALSE;
		}
	}

	return PyObject_TypeCheck (py_obj, _PyGtkWidget_Type) ? TRUE : FALSE;
}

static void
impl_update_ui (EomPlugin *plugin,
		EomWindow *window)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();

	EomPythonPlugin *pyplugin = (EomPythonPlugin *) plugin;

	if (PyObject_HasAttrString (pyplugin->instance, "update_ui")) {
		PyObject *py_ret = call_python_method (pyplugin, window, "update_ui");

		if (py_ret)
		{
			Py_XDECREF (py_ret);
		}
	} else {
		EOM_PLUGIN_CLASS (parent_class)->update_ui (plugin, window);
	}

	pyg_gil_state_release (state);
}

static void
impl_deactivate (EomPlugin *plugin,
		 EomWindow *window)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();

	EomPythonPlugin *pyplugin = (EomPythonPlugin *) plugin;

	if (PyObject_HasAttrString (pyplugin->instance, "deactivate")) {
		PyObject *py_ret = call_python_method (pyplugin, window, "deactivate");

		if (py_ret) {
			Py_XDECREF (py_ret);
		}
	} else {
		EOM_PLUGIN_CLASS (parent_class)->deactivate (plugin, window);
	}

	pyg_gil_state_release (state);
}

static void
impl_activate (EomPlugin *plugin,
	       EomWindow *window)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();

	EomPythonPlugin *pyplugin = (EomPythonPlugin *) plugin;

	if (PyObject_HasAttrString (pyplugin->instance, "activate")) {
		PyObject *py_ret = call_python_method (pyplugin, window, "activate");

		if (py_ret) {
			Py_XDECREF (py_ret);
		}
	} else {
		EOM_PLUGIN_CLASS (parent_class)->activate (plugin, window);
	}

	pyg_gil_state_release (state);
}

static GtkWidget *
impl_create_configure_dialog (EomPlugin *plugin)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();
	EomPythonPlugin *pyplugin = (EomPythonPlugin *) plugin;
	GtkWidget *ret = NULL;

	if (PyObject_HasAttrString (pyplugin->instance, "create_configure_dialog")) {
		PyObject *py_ret = call_python_method (pyplugin, NULL, "create_configure_dialog");

		if (py_ret) {
			if (check_py_object_is_gtk_widget (py_ret)) {
				ret = GTK_WIDGET (pygobject_get (py_ret));
				g_object_ref (ret);
			} else {
				PyErr_SetString(PyExc_TypeError, "Return value for create_configure_dialog is not a GtkWidget");
				PyErr_Print();
			}

			Py_DECREF (py_ret);
		}
	} else {
		ret = EOM_PLUGIN_CLASS (parent_class)->create_configure_dialog (plugin);
	}

	pyg_gil_state_release (state);

	return ret;
}

static gboolean
impl_is_configurable (EomPlugin *plugin)
{
	PyGILState_STATE state = pyg_gil_state_ensure ();

	EomPythonPlugin *pyplugin = (EomPythonPlugin *) plugin;

	PyObject *dict = pyplugin->instance->ob_type->tp_dict;

	gboolean result;

	if (dict == NULL)
		result = FALSE;
	else if (!PyDict_Check(dict))
		result = FALSE;
	else
		result = PyDict_GetItemString(dict, "create_configure_dialog") != NULL;

	pyg_gil_state_release (state);

	return result;
}

static void
eom_python_plugin_init (EomPythonPlugin *plugin)
{
	EomPythonPluginClass *class;

	eom_debug_message (DEBUG_PLUGINS, "Creating Python plugin instance");

	class = (EomPythonPluginClass*) (((GTypeInstance*) plugin)->g_class);

	plugin->instance = PyObject_CallObject (class->type, NULL);

	if (plugin->instance == NULL)
		PyErr_Print();
}

static void
eom_python_plugin_finalize (GObject *plugin)
{
	eom_debug_message (DEBUG_PLUGINS, "Finalizing Python plugin instance");

	Py_DECREF (((EomPythonPlugin *) plugin)->instance);

	G_OBJECT_CLASS (parent_class)->finalize (plugin);
}

static void
eom_python_plugin_class_init (EomPythonPluginClass *klass,
				gpointer                class_data)
{
	EomPluginClass *plugin_class = EOM_PLUGIN_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	klass->type = (PyObject*) class_data;

	G_OBJECT_CLASS (klass)->finalize = eom_python_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;
	plugin_class->create_configure_dialog = impl_create_configure_dialog;
	plugin_class->is_configurable = impl_is_configurable;
}

GType
eom_python_plugin_get_type (GTypeModule *module,
			    PyObject    *type)
{
	GType gtype;
	gchar *type_name;

	GTypeInfo info = {
		sizeof (EomPythonPluginClass),
		NULL,           /* base_init */
		NULL,           /* base_finalize */
		(GClassInitFunc) eom_python_plugin_class_init,
		NULL,           /* class_finalize */
		type,           /* class_data */
		sizeof (EomPythonPlugin),
		0,              /* n_preallocs */
		(GInstanceInitFunc) eom_python_plugin_init,
	};

	Py_INCREF (type);

	type_name = g_strdup_printf ("%s+EomPythonPlugin",
				     PyString_AsString (PyObject_GetAttrString (type, "__name__")));

	eom_debug_message (DEBUG_PLUGINS, "Registering Python plugin instance: %s", type_name);

	gtype = g_type_module_register_type (module,
					     EOM_TYPE_PLUGIN,
					     type_name,
					     &info, 0);

	g_free (type_name);

	return gtype;
}
