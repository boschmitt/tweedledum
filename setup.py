# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
import os
import sys
import skbuild
import setuptools

build_dev_version = os.environ.get("TWEEDLEDUM_DEV_VERSION", False)


def get_name():
    if build_dev_version:
        return "tweedledum-dev"
    else:
        return "tweedledum"


def get_version():
    if build_dev_version:
        return "1.2.0.dev" + str(build_dev_version)
    else:
        return "1.1.0"


skbuild.setup(
    name=get_name(),
    version=get_version(),
    package_dir={"": "python"},
    packages=setuptools.find_packages(
        where="python", include=["tweedledum", "tweedledum.*"]
    ),
    include_package_data=True,
    zip_safe=False,
    python_requires=">=3.6",
    cmake_install_dir="python",
)

# /!\ This is a 'gambiarra':
# For whatever reason, whenever I try to install `tweedledum` in development
# mode, i.e., `pip install -e .`, the easy_path and egg-link are wrong! Hence,
# I basically fix them by hand because I couldn't figure out a better way of
# doing it!
if len(sys.argv) > 1 and sys.argv[1] == "develop":
    from pathlib import Path
    import fileinput
    import shutil
    import site

    easy_install = Path(site.getsitepackages()[0]) / "easy-install.pth"
    egg_link = Path(site.getsitepackages()[0]) / "tweedledum.egg-link"
    for line in fileinput.input([easy_install, egg_link], inplace=True):
        print(line.replace("tweedledum", "tweedledum/python"), end="")
    shutil.rmtree("tweedledum.egg-info", ignore_errors=True)
