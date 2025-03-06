# Documentation build configuration file.
# Reference: https://www.sphinx-doc.org/en/master/usage/configuration.html

import subprocess

subprocess.call("doxygen _doxygen/Doxyfile", shell=True)

# -- Project --------------------------------------------------------------

project = "NTU Racing Team Zephyr Common"
copyright = "2025, NTU Racing Team"
author = "NTU Racing Team members"

# -- General configuration ------------------------------------------------

extensions = [
    "breathe",
    "sphinx.ext.autodoc",
    "sphinx.ext.ifconfig",
    "sphinx.ext.mathjax",
    "sphinx.ext.todo",
    "sphinx.ext.viewcode",
    "sphinx_rtd_theme",
]

templates_path = ["_templates"]

exclude_patterns = ["_build"]

# -- Options for HTML output ----------------------------------------------

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "logo_only": True,
    "prev_next_buttons_location": "bottom",
}
html_title = "NTU Racing Team Zephyr Common Documentation"
html_logo = "_static/images/logo.png"
html_static_path = ["_static"]

github_url = "https://github.com/NTURacingTeam/nturt_zephyr_common"

# -- Options for Breathe plugin -------------------------------------------

breathe_projects = {
	"NTURT Zephyr Common Library": "_build_doxygen/xml/"
}
breathe_default_project = "NTURT Zephyr Common Library"
breathe_domain_by_extension = {
    "h": "c",
    "c": "c",
}
breathe_show_enumvalue_initializer = True
breathe_default_members = ("members", )
