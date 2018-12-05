# Minimal makefile for Sphinx documentation

VENV_NAME     ?= .venv
VENV_ACTIVATE  = $(VENV_NAME)/bin/activate
PYTHON         = ${VENV_NAME}/bin/python3

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = ${PYTHON} $(VENV_NAME)/bin/sphinx-build
SPHINXPROJ    = tweedledum
SOURCEDIR     = .
BUILDDIR      = _build

venv: $(VENV_NAME)/bin/activate
$(VENV_NAME)/bin/activate:
	test -d $(VENV_NAME) || virtualenv -p python3 $(VENV_NAME)
	${PYTHON} -m pip install --upgrade -r requirements.txt
	touch $(VENV_NAME)/bin/activate

doxygen:
	doxygen Doxyfile

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile venv doxygen

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile venv doxygen
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)