#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import os
import sys
import setuptools
import time
from pathlib import Path

build_dev_version = os.environ.get('TWEEDLEDUM_DEV_VERSION', False)

try:
    import skbuild
except ImportError:
    print('scikit-build is required to build from source.', file=sys.stderr)
    print('Please run:', file=sys.stderr)
    print('', file=sys.stderr)
    print('  python -m pip install scikit-build')
    sys.exit(1)

README_PATH = os.path.join(os.path.abspath(os.path.dirname(__file__)),
                           'README.md')
with open(README_PATH) as readme_file:
    README = readme_file.read()

def get_name():
    if build_dev_version:
        return "tweedledum-dev"
    else:
        return "tweedledum"

def get_version():
    if build_dev_version:
        return "1.1.0.dev" + str(build_dev_version)
    else:
        return "1.0.0"

skbuild.setup(
    name=get_name(),
    version=get_version(),
    description="A library for synthesizing and manipulating quantum circuits",
    long_description=README,
    long_description_content_type='text/markdown',
    url="https://github.com/boschmitt/tweedledum",
    author='Bruno Schmitt',
    author_email='bruno.schmitt@epfl.ch',
    license="MIT",
    package_dir={'': 'python'},
    packages=setuptools.find_packages(where='python', include=['tweedledum', 'tweedledum.*']),
    include_package_data=True,
    zip_safe=False,
    python_requires=">=3.6",
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: MacOS",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: C++",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9"
    ],
    cmake_install_dir='python'
)

# /!\ This is a 'gambiarra':
# For whatever reason, whenever I try to install `tweedledum` in development 
# mode, i.e., `pip install -e .`, the easy_path and egg-link are wrong! Hence, 
# I basically fix them by hand because I couldn't figure out a better way of 
# doing it!
if len(sys.argv) > 1 and sys.argv[1] == "develop":
    import fileinput
    import shutil
    import site
    easy_install = Path(site.getsitepackages()[0]) / 'easy-install.pth'
    egg_link = Path(site.getsitepackages()[0])  / 'tweedledum.egg-link'
    for line in fileinput.input([easy_install, egg_link], inplace=True):
        print(line.replace("tweedledum", "tweedledum/python"),  end="")
    shutil.rmtree('tweedledum.egg-info', ignore_errors=True)
