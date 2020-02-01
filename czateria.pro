TEMPLATE = subdirs
SUBDIRS = czatlib ui czatlibtest

ui.depends = czatlib
czatlibtest.depends = czatlib
